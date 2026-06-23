#include <Windows.h>
#include <mmsystem.h>
#include "skVideoView.h"
#include "skTheme.h"
#include "skTypeface.h"
#include <include/core/SkPaint.h>
#include <include/core/SkFont.h>
#include <include/core/SkCanvas.h>
#include <include/core/SkRRect.h>
#include <include/core/SkPath.h>
#include <include/core/SkImage.h>
#include <include/core/SkPixmap.h>
#include <include/core/SkSamplingOptions.h>
#include <algorithm>
#include <atomic>
#include <chrono>
#include <condition_variable>
#include <cstdio>
#include <cstring>
#include <memory>
#include <mutex>
#include <thread>
#include <vector>

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavutil/error.h>
#include <libavutil/imgutils.h>
#include <libavutil/time.h>
#include <libswresample/swresample.h>
#include <libswscale/swscale.h>
}

static sk_sp<SkTypeface>& vvTf() {
    static sk_sp<SkTypeface> tf = skGetSystemTypeface();
    return tf;
}

static std::string ffError(int code) {
    char buffer[AV_ERROR_MAX_STRING_SIZE] = {};
    av_strerror(code, buffer, sizeof(buffer));
    return buffer;
}

struct skVideoView::Impl {
    struct AudioBuffer {
        WAVEHDR header {};
        std::vector<uint8_t> bytes;
    };

    AVFormatContext* format = nullptr;
    AVCodecContext* videoCodec = nullptr;
    AVCodecContext* audioCodec = nullptr;
    SwsContext* scaler = nullptr;
    SwrContext* resampler = nullptr;
    int videoStream = -1;
    int audioStream = -1;

    std::thread decodeThread;
    std::atomic<bool> quit { false };
    std::atomic<bool> playing { false };
    std::atomic<bool> ended { false };
    std::atomic<double> position { 0.0 };
    std::atomic<double> duration { 0.0 };
    std::atomic<double> seekRequest { -1.0 };

    std::mutex imageMutex;
    sk_sp<SkImage> image;
    std::mutex errorMutex;
    std::string error;

    HWAVEOUT waveOut = nullptr;
    std::mutex audioMutex;
    std::vector<std::unique_ptr<AudioBuffer>> audioBuffers;
    static constexpr int kAudioRate = 48000;
    static constexpr int kAudioChannels = 2;

    ~Impl() { close(); }

    void setError(const std::string& message) {
        std::lock_guard<std::mutex> lock(errorMutex);
        error = message;
    }

    std::string getError() {
        std::lock_guard<std::mutex> lock(errorMutex);
        return error;
    }

    static AVCodecContext* openCodec(AVFormatContext* fmt, int streamIndex, std::string& errorOut) {
        if (streamIndex < 0) return nullptr;
        AVStream* stream = fmt->streams[streamIndex];
        const AVCodec* codec = avcodec_find_decoder(stream->codecpar->codec_id);
        if (!codec) {
            errorOut = "No decoder is available for this media stream";
            return nullptr;
        }
        AVCodecContext* context = avcodec_alloc_context3(codec);
        if (!context) {
            errorOut = "Could not allocate the decoder";
            return nullptr;
        }
        int result = avcodec_parameters_to_context(context, stream->codecpar);
        if (result >= 0) result = avcodec_open2(context, codec, nullptr);
        if (result < 0) {
            errorOut = ffError(result);
            avcodec_free_context(&context);
            return nullptr;
        }
        return context;
    }

    bool open(const std::string& path) {
        close();
        quit = false;
        ended = false;
        position = 0.0;
        duration = 0.0;
        seekRequest = -1.0;
        setError({});

        int result = avformat_open_input(&format, path.c_str(), nullptr, nullptr);
        if (result < 0) {
            setError("Could not open video: " + ffError(result));
            return false;
        }
        result = avformat_find_stream_info(format, nullptr);
        if (result < 0) {
            setError("Could not read media streams: " + ffError(result));
            closeMedia();
            return false;
        }

        if (format->duration != AV_NOPTS_VALUE)
            duration = static_cast<double>(format->duration) / AV_TIME_BASE;

        videoStream = av_find_best_stream(format, AVMEDIA_TYPE_VIDEO, -1, -1, nullptr, 0);
        audioStream = av_find_best_stream(format, AVMEDIA_TYPE_AUDIO, -1, videoStream, nullptr, 0);
        if (videoStream < 0) {
            setError("The file does not contain a decodable video stream");
            closeMedia();
            return false;
        }

        std::string codecError;
        videoCodec = openCodec(format, videoStream, codecError);
        if (!videoCodec) {
            setError("Could not initialize video decoder: " + codecError);
            closeMedia();
            return false;
        }

        if (audioStream >= 0) {
            audioCodec = openCodec(format, audioStream, codecError);
            if (audioCodec) initializeAudio();
            else audioStream = -1; // Video remains usable when audio is unsupported.
        }

        decodeThread = std::thread([this] { decodeLoop(); });
        seekRequest = 0.0; // Decode the first frame while stopped.
        return true;
    }

    void initializeAudio() {
        AVChannelLayout outputLayout = AV_CHANNEL_LAYOUT_STEREO;
        int result = swr_alloc_set_opts2(&resampler,
            &outputLayout, AV_SAMPLE_FMT_S16, kAudioRate,
            &audioCodec->ch_layout, audioCodec->sample_fmt, audioCodec->sample_rate,
            0, nullptr);
        av_channel_layout_uninit(&outputLayout);
        if (result < 0 || !resampler || swr_init(resampler) < 0) {
            swr_free(&resampler);
            avcodec_free_context(&audioCodec);
            audioStream = -1;
            return;
        }

        WAVEFORMATEX waveFormat {};
        waveFormat.wFormatTag = WAVE_FORMAT_PCM;
        waveFormat.nChannels = kAudioChannels;
        waveFormat.nSamplesPerSec = kAudioRate;
        waveFormat.wBitsPerSample = 16;
        waveFormat.nBlockAlign = waveFormat.nChannels * waveFormat.wBitsPerSample / 8;
        waveFormat.nAvgBytesPerSec = waveFormat.nSamplesPerSec * waveFormat.nBlockAlign;
        if (waveOutOpen(&waveOut, WAVE_MAPPER, &waveFormat, 0, 0, CALLBACK_NULL) != MMSYSERR_NOERROR) {
            waveOut = nullptr;
            swr_free(&resampler);
            avcodec_free_context(&audioCodec);
            audioStream = -1;
        }
    }

    void cleanupAudioBuffersLocked() {
        auto it = audioBuffers.begin();
        while (it != audioBuffers.end()) {
            if (((*it)->header.dwFlags & WHDR_DONE) != 0) {
                waveOutUnprepareHeader(waveOut, &(*it)->header, sizeof(WAVEHDR));
                it = audioBuffers.erase(it);
            } else {
                ++it;
            }
        }
    }

    void resetAudio() {
        std::lock_guard<std::mutex> lock(audioMutex);
        if (!waveOut) return;
        waveOutReset(waveOut);
        for (auto& buffer : audioBuffers)
            waveOutUnprepareHeader(waveOut, &buffer->header, sizeof(WAVEHDR));
        audioBuffers.clear();
    }

    bool audioPending() {
        std::lock_guard<std::mutex> lock(audioMutex);
        if (!waveOut) return false;
        cleanupAudioBuffersLocked();
        return !audioBuffers.empty();
    }

    void queueAudio(AVFrame* frame) {
        if (!waveOut || !resampler || !playing) return;
        const int maxSamples = static_cast<int>(av_rescale_rnd(
            swr_get_delay(resampler, audioCodec->sample_rate) + frame->nb_samples,
            kAudioRate, audioCodec->sample_rate, AV_ROUND_UP));
        if (maxSamples <= 0) return;

        auto buffer = std::make_unique<AudioBuffer>();
        buffer->bytes.resize(static_cast<size_t>(maxSamples) * kAudioChannels * sizeof(int16_t));
        uint8_t* output[] = { buffer->bytes.data() };
        const int samples = swr_convert(resampler, output, maxSamples,
            const_cast<const uint8_t**>(frame->extended_data), frame->nb_samples);
        if (samples <= 0) return;
        buffer->bytes.resize(static_cast<size_t>(samples) * kAudioChannels * sizeof(int16_t));
        buffer->header.lpData = reinterpret_cast<LPSTR>(buffer->bytes.data());
        buffer->header.dwBufferLength = static_cast<DWORD>(buffer->bytes.size());

        for (;;) {
            {
                std::lock_guard<std::mutex> lock(audioMutex);
                cleanupAudioBuffersLocked();
                if (audioBuffers.size() < 8) {
                    if (waveOutPrepareHeader(waveOut, &buffer->header, sizeof(WAVEHDR)) == MMSYSERR_NOERROR &&
                        waveOutWrite(waveOut, &buffer->header, sizeof(WAVEHDR)) == MMSYSERR_NOERROR) {
                        audioBuffers.push_back(std::move(buffer));
                    }
                    return;
                }
            }
            if (quit || !playing) return;
            std::this_thread::sleep_for(std::chrono::milliseconds(5));
        }
    }

    void publishVideoFrame(AVFrame* frame) {
        scaler = sws_getCachedContext(scaler,
            frame->width, frame->height, static_cast<AVPixelFormat>(frame->format),
            frame->width, frame->height, AV_PIX_FMT_BGRA,
            SWS_BILINEAR, nullptr, nullptr, nullptr);
        if (!scaler) return;

        std::vector<uint8_t> pixels(static_cast<size_t>(frame->width) * frame->height * 4);
        uint8_t* destination[] = { pixels.data() };
        int lines[] = { frame->width * 4 };
        sws_scale(scaler, frame->data, frame->linesize, 0, frame->height, destination, lines);

        SkImageInfo info = SkImageInfo::Make(frame->width, frame->height,
                                             kBGRA_8888_SkColorType, kOpaque_SkAlphaType);
        SkPixmap pixmap(info, pixels.data(), static_cast<size_t>(lines[0]));
        auto decoded = SkImages::RasterFromPixmapCopy(pixmap);
        if (decoded) {
            std::lock_guard<std::mutex> lock(imageMutex);
            image = std::move(decoded);
        }
    }

    void performSeek(double seconds) {
        const int64_t timestamp = static_cast<int64_t>(seconds * AV_TIME_BASE);
        if (av_seek_frame(format, -1, timestamp, AVSEEK_FLAG_BACKWARD) >= 0) {
            avcodec_flush_buffers(videoCodec);
            if (audioCodec) avcodec_flush_buffers(audioCodec);
            if (resampler) {
                swr_close(resampler);
                swr_init(resampler);
            }
            resetAudio();
            position = seconds;
            ended = false;
        }
    }

    void decodeLoop() {
        AVPacket* packet = av_packet_alloc();
        AVFrame* frame = av_frame_alloc();
        bool previewFrame = true;
        bool wasPlaying = false;
        double discardBefore = 0.0;
        auto clockBase = std::chrono::steady_clock::now();

        while (!quit) {
            const double requested = seekRequest.exchange(-1.0);
            if (requested >= 0.0) {
                performSeek(requested);
                discardBefore = requested;
                previewFrame = true;
                clockBase = std::chrono::steady_clock::now() -
                    std::chrono::milliseconds(static_cast<int64_t>(requested * 1000.0));
            }

            const bool isPlaying = playing.load();
            if (isPlaying && !wasPlaying) {
                clockBase = std::chrono::steady_clock::now() -
                    std::chrono::milliseconds(static_cast<int64_t>(position.load() * 1000.0));
            }
            wasPlaying = isPlaying;
            if (!isPlaying && !previewFrame) {
                std::this_thread::sleep_for(std::chrono::milliseconds(10));
                continue;
            }

            int result = av_read_frame(format, packet);
            if (result < 0) {
                while (!quit && playing && audioPending())
                    std::this_thread::sleep_for(std::chrono::milliseconds(5));
                playing = false;
                ended = true;
                position = duration.load();
                break;
            }

            if (packet->stream_index == videoStream) {
                result = avcodec_send_packet(videoCodec, packet);
                while (result >= 0 && avcodec_receive_frame(videoCodec, frame) >= 0) {
                    double pts = position.load();
                    if (frame->best_effort_timestamp != AV_NOPTS_VALUE)
                        pts = frame->best_effort_timestamp * av_q2d(format->streams[videoStream]->time_base);
                    if (pts + 0.001 < discardBefore) continue;
                    discardBefore = 0.0;

                    // With audio, the bounded waveOut queue provides the media
                    // clock. Video-only files use a wall clock here.
                    if (playing && !waveOut) {
                        auto target = clockBase +
                            std::chrono::milliseconds(static_cast<int64_t>(pts * 1000.0));
                        while (!quit && playing && std::chrono::steady_clock::now() < target)
                            std::this_thread::sleep_for(std::chrono::milliseconds(2));
                    }
                    publishVideoFrame(frame);
                    position = pts;
                    if (previewFrame && !playing) previewFrame = false;
                }
            } else if (packet->stream_index == audioStream && audioCodec && playing) {
                result = avcodec_send_packet(audioCodec, packet);
                while (result >= 0 && avcodec_receive_frame(audioCodec, frame) >= 0)
                    queueAudio(frame);
            }
            av_packet_unref(packet);
        }

        av_frame_free(&frame);
        av_packet_free(&packet);
    }

    void setPlaying(bool value) {
        playing = value;
        std::lock_guard<std::mutex> lock(audioMutex);
        if (waveOut) {
            if (value) waveOutRestart(waveOut);
            else waveOutPause(waveOut);
        }
    }

    void seek(double seconds) {
        seekRequest = std::max(0.0, std::min(duration.load(), seconds));
        position = seekRequest.load();
    }

    void closeMedia() {
        sws_freeContext(scaler); scaler = nullptr;
        swr_free(&resampler);
        avcodec_free_context(&audioCodec);
        avcodec_free_context(&videoCodec);
        avformat_close_input(&format);
        videoStream = audioStream = -1;
    }

    void close() {
        quit = true;
        playing = false;
        if (decodeThread.joinable()) decodeThread.join();
        resetAudio();
        if (waveOut) {
            waveOutClose(waveOut);
            waveOut = nullptr;
        }
        closeMedia();
        std::lock_guard<std::mutex> lock(imageMutex);
        image.reset();
    }
};

skVideoView::skVideoView(int sx, int sy, int sw, int sh)
    : skWidget(sx, sy, sw, sh), m_impl(std::make_shared<Impl>()) {}

skVideoView::~skVideoView() = default;

void skVideoView::loadFile(const std::string& path) {
    m_filepath = path;
    m_filename = path;
    auto pos = path.find_last_of("/\\");
    if (pos != std::string::npos) m_filename = path.substr(pos+1);
    m_position = 0.f;
    m_duration = 0.f;
    m_state = State::Stopped;
    m_impl->open(path);
    m_duration = static_cast<float>(m_impl->duration.load());
}

void skVideoView::play() {
    if (m_filepath.empty() || !m_impl->format) return;
    if (m_position >= m_duration && m_duration > 0.f) seek(0.f);
    m_state = State::Playing;
    m_impl->setPlaying(true);
    if (m_onStateChange) m_onStateChange(true);
}

void skVideoView::pause() {
    if (m_state == State::Stopped) return;
    m_state = State::Paused;
    m_impl->setPlaying(false);
    if (m_onStateChange) m_onStateChange(false);
}

void skVideoView::stop() {
    m_state = State::Stopped;
    m_impl->setPlaying(false);
    seek(0.f);
    if (m_onStateChange) m_onStateChange(false);
}

void skVideoView::seek(float seconds) {
    const float limit = m_duration > 0.f ? m_duration : static_cast<float>(m_impl->duration.load());
    m_position = std::max(0.f, std::min(limit, seconds));
    m_impl->seek(m_position);
}

void skVideoView::onTick() {
    ++m_tickCount;
    m_position = static_cast<float>(m_impl->position.load());
    const float decodedDuration = static_cast<float>(m_impl->duration.load());
    if (decodedDuration > 0.f) m_duration = decodedDuration;
    if (m_impl->ended.exchange(false)) {
        m_state = State::Stopped;
        if (m_onStateChange) m_onStateChange(false);
        if (m_onEnd) m_onEnd();
    }
}

SkRect skVideoView::frameRect()  const { return SkRect::MakeXYWH((float)x,(float)y,(float)w,(float)std::max(0,h-kCtrlH)); }
SkRect skVideoView::seekRect()   const { return SkRect::MakeXYWH((float)(x+kSeekPad),(float)(y+h-kCtrlH+kSeekPad),(float)std::max(0,w-2*kSeekPad),(float)kSeekH); }
SkRect skVideoView::playBtn()    const { return SkRect::MakeXYWH((float)(x+4),(float)(y+h-kCtrlH+4),(float)kBtnW,(float)kBtnH); }
SkRect skVideoView::pauseBtn()   const { SkRect p=playBtn(); return SkRect::MakeXYWH(p.right()+4.f,p.top(),(float)kBtnW,(float)kBtnH); }
SkRect skVideoView::stopBtn()    const { SkRect p=pauseBtn(); return SkRect::MakeXYWH(p.right()+4.f,p.top(),(float)kBtnW,(float)kBtnH); }

static void drawSymbol(SkCanvas* canvas, SkRect rect, const char* symbol, SkColor color, const SkFont& font) {
    SkPaint paint; paint.setAntiAlias(true); paint.setColor(color);
    SkRect bounds; font.measureText(symbol,strlen(symbol),SkTextEncoding::kUTF8,&bounds);
    canvas->drawString(symbol, rect.centerX()-bounds.width()/2.f-bounds.left(),
                       rect.centerY()-bounds.height()/2.f-bounds.top(), font, paint);
}

void skVideoView::Paint(SkCanvas* canvas) {
    const auto& th = skGetTheme();
    SkFont font(vvTf(), 11.f); font.setEdging(SkFont::Edging::kAntiAlias);
    SkFont smallFont(vvTf(), 10.f); smallFont.setEdging(SkFont::Edging::kAntiAlias);

    SkRRect outerRR; outerRR.setRectXY(SkRect::MakeXYWH((float)x,(float)y,(float)w,(float)h),6.f,6.f);
    SkPaint background; background.setAntiAlias(true); background.setColor(SkColorSetRGB(12,12,16));
    canvas->save(); canvas->clipRRect(outerRR, SkClipOp::kIntersect, true);
    canvas->drawRRect(outerRR, background);

    SkRect frame = frameRect();
    SkPaint frameBackground; frameBackground.setColor(SkColorSetRGB(8,8,12));
    canvas->drawRect(frame,frameBackground);

    sk_sp<SkImage> image;
    {
        std::lock_guard<std::mutex> lock(m_impl->imageMutex);
        image = m_impl->image;
    }
    if (image) {
        const float scale = std::min(frame.width()/image->width(), frame.height()/image->height());
        const float drawWidth = image->width()*scale, drawHeight = image->height()*scale;
        SkRect destination = SkRect::MakeXYWH(frame.centerX()-drawWidth/2.f,
            frame.centerY()-drawHeight/2.f, drawWidth, drawHeight);
        canvas->drawImageRect(image.get(), destination, SkSamplingOptions(SkFilterMode::kLinear));
    } else {
        const std::string error = m_impl->getError();
        const char* message = !error.empty() ? error.c_str() :
            (m_filename.empty() ? "No video loaded" : "Loading video...");
        SkPaint text; text.setAntiAlias(true);
        text.setColor(error.empty() ? SkColorSetRGB(100,100,120) : SkColorSetRGB(220,90,90));
        canvas->drawString(message, frame.left()+10.f, frame.centerY(), smallFont, text);
    }

    SkRect controls = SkRect::MakeXYWH((float)x,(float)(y+h-kCtrlH),(float)w,(float)kCtrlH);
    SkPaint controlsPaint; controlsPaint.setColor(SkColorSetRGB(20,20,26));
    canvas->drawRect(controls,controlsPaint);

    SkRect seekTrack = seekRect();
    SkPaint track; track.setAntiAlias(true); track.setColor(SkColorSetRGB(50,50,60));
    SkRRect trackRR; trackRR.setRectXY(seekTrack,2.f,2.f); canvas->drawRRect(trackRR,track);
    if (m_duration > 0.f) {
        const float ratio = std::max(0.f, std::min(1.f, m_position/m_duration));
        SkRect fill = SkRect::MakeXYWH(seekTrack.left(),seekTrack.top(),seekTrack.width()*ratio,(float)kSeekH);
        SkPaint fillPaint; fillPaint.setAntiAlias(true); fillPaint.setColor(th.accent);
        SkRRect fillRR; fillRR.setRectXY(fill,2.f,2.f); canvas->drawRRect(fillRR,fillPaint);
        SkPaint thumb; thumb.setAntiAlias(true); thumb.setColor(SK_ColorWHITE);
        canvas->drawCircle(seekTrack.left()+seekTrack.width()*ratio,seekTrack.centerY(),5.f,thumb);
    }

    auto drawButton = [&](SkRect rect, const char* symbol, bool hovered) {
        SkRRect buttonRR; buttonRR.setRectXY(rect,4.f,4.f);
        SkPaint button; button.setAntiAlias(true);
        button.setColor(hovered ? SkColorSetARGB(60,255,255,255) : SkColorSetARGB(30,255,255,255));
        canvas->drawRRect(buttonRR,button);
        drawSymbol(canvas, rect, symbol, SK_ColorWHITE, font);
    };
    drawButton(playBtn(),  "\xe2\x96\xb6", m_playHov);
    drawButton(pauseBtn(), "\xe2\x80\x96", m_pauseHov);
    drawButton(stopBtn(),  "\xe2\x96\xa0", m_stopHov);

    char timeBuffer[32];
    const int current = (int)m_position, total = (int)m_duration;
    snprintf(timeBuffer,sizeof(timeBuffer),"%d:%02d / %d:%02d", current/60,current%60,total/60,total%60);
    SkPaint timePaint; timePaint.setAntiAlias(true); timePaint.setColor(SkColorSetRGB(160,160,180));
    canvas->drawString(timeBuffer, stopBtn().right()+8.f, controls.centerY()+4.f, smallFont, timePaint);

    SkPaint border; border.setAntiAlias(true); border.setStyle(SkPaint::kStroke_Style);
    border.setStrokeWidth(0.75f); border.setColor(th.panelBorder); canvas->drawRRect(outerRR,border);
    canvas->restore();
}

void skVideoView::OnEvent(const skEvent& ev) {
    if (ev.type == skEventType::MouseMove) {
        m_playHov  = playBtn().contains((float)ev.x,(float)ev.y);
        m_pauseHov = pauseBtn().contains((float)ev.x,(float)ev.y);
        m_stopHov  = stopBtn().contains((float)ev.x,(float)ev.y);
        if (m_seekDrag && m_duration > 0.f) {
            SkRect track = seekRect();
            float ratio = ((float)ev.x-track.left())/std::max(1.f,track.width());
            seek(std::max(0.f,std::min(1.f,ratio))*m_duration);
        }
    }
    if (ev.type == skEventType::MouseDown) {
        if (playBtn().contains((float)ev.x,(float)ev.y))  { play(); return; }
        if (pauseBtn().contains((float)ev.x,(float)ev.y)) { pause(); return; }
        if (stopBtn().contains((float)ev.x,(float)ev.y))  { stop(); return; }
        SkRect track = seekRect();
        if (track.makeOutset(0.f,6.f).contains((float)ev.x,(float)ev.y) && m_duration > 0.f) {
            m_seekDrag = true;
            float ratio = ((float)ev.x-track.left())/std::max(1.f,track.width());
            seek(std::max(0.f,std::min(1.f,ratio))*m_duration);
        }
    }
    if (ev.type == skEventType::MouseUp || ev.type == skEventType::MouseCancel)
        m_seekDrag = false;
}
