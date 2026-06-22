#pragma once
#include "skWidget.h"
#include <string>
#include <functional>

// Video player widget. Currently renders a placeholder frame area with
// simulated playback state. Wire setOnFrame() to receive per-tick callbacks
// when a real decoder is available.
class skVideoView : public skWidget {
public:
    skVideoView(int x, int y, int w, int h);

    void loadFile(const std::string& path);
    void play();
    void pause();
    void stop();
    void seek(float seconds);

    bool        isPlaying()  const { return m_state == State::Playing; }
    float       position()   const { return m_position; }
    float       duration()   const { return m_duration; }
    std::string filename()   const { return m_filename; }

    void setDuration(float secs) { m_duration = secs; }
    void setOnStateChange(std::function<void(bool playing)> fn) { m_onStateChange = std::move(fn); }
    void setOnEnd(std::function<void()> fn)                     { m_onEnd         = std::move(fn); }

    void Paint(SkCanvas* canvas) override;
    void OnEvent(const skEvent& ev) override;
    void onTick() override;

private:
    enum class State { Stopped, Playing, Paused };

    std::string m_filepath;
    std::string m_filename;
    State       m_state    = State::Stopped;
    float       m_position = 0.f;
    float       m_duration = 0.f;
    int         m_tickCount = 0; // for animated icon
    bool        m_playHov = false, m_pauseHov = false, m_stopHov = false;
    bool        m_seekDrag = false;

    std::function<void(bool)> m_onStateChange;
    std::function<void()>     m_onEnd;

    static constexpr int kCtrlH   = 34; // control strip height
    static constexpr int kBtnW    = 36;
    static constexpr int kBtnH    = 26;
    static constexpr int kSeekH   = 4;
    static constexpr int kSeekPad = 6;

    // Returns the rect of the frame area (above controls)
    SkRect frameRect()  const;
    SkRect seekRect()   const;
    SkRect playBtn()    const;
    SkRect pauseBtn()   const;
    SkRect stopBtn()    const;
};
