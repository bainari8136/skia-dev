#include "skVideoView.h"
#include "skTheme.h"
#include "skTypeface.h"
#include <include/core/SkPaint.h>
#include <include/core/SkFont.h>
#include <include/core/SkCanvas.h>
#include <include/core/SkRRect.h>
#include <include/core/SkPath.h>
#include <algorithm>
#include <cstdio>
#include <cstring>

static sk_sp<SkTypeface>& vvTf() {
    static sk_sp<SkTypeface> tf = skGetSystemTypeface();
    return tf;
}

skVideoView::skVideoView(int sx, int sy, int sw, int sh)
    : skWidget(sx, sy, sw, sh) {}

void skVideoView::loadFile(const std::string& path) {
    m_filepath = path;
    m_filename = path;
    auto pos = path.find_last_of("/\\");
    if (pos != std::string::npos) m_filename = path.substr(pos+1);
    m_position = 0.f;
    m_state    = State::Stopped;
    if (m_duration <= 0.f) m_duration = 120.f;
}

void skVideoView::play() {
    if (m_filepath.empty()) return;
    m_state = State::Playing;
    if (m_onStateChange) m_onStateChange(true);
}
void skVideoView::pause() {
    m_state = State::Paused;
    if (m_onStateChange) m_onStateChange(false);
}
void skVideoView::stop() {
    m_state = State::Stopped;
    m_position = 0.f;
    if (m_onStateChange) m_onStateChange(false);
}
void skVideoView::seek(float secs) {
    m_position = std::max(0.f, std::min(m_duration, secs));
}

void skVideoView::onTick() {
    ++m_tickCount;
    if (m_state == State::Playing && m_duration > 0.f) {
        m_position += 0.1f; // 100 ms per tick
        if (m_position >= m_duration) {
            m_position = m_duration;
            m_state    = State::Stopped;
            if (m_onEnd) m_onEnd();
        }
    }
}

SkRect skVideoView::frameRect()  const { return SkRect::MakeXYWH((float)x,(float)y,(float)w,(float)(h-kCtrlH)); }
SkRect skVideoView::seekRect()   const { return SkRect::MakeXYWH((float)(x+kSeekPad),(float)(y+h-kCtrlH+kSeekPad),(float)(w-2*kSeekPad),(float)kSeekH); }
SkRect skVideoView::playBtn()    const { return SkRect::MakeXYWH((float)(x+4),(float)(y+h-kCtrlH+4),(float)kBtnW,(float)kBtnH); }
SkRect skVideoView::pauseBtn()   const { SkRect p=playBtn(); return SkRect::MakeXYWH(p.right()+4.f,p.top(),(float)kBtnW,(float)kBtnH); }
SkRect skVideoView::stopBtn()    const { SkRect p=pauseBtn(); return SkRect::MakeXYWH(p.right()+4.f,p.top(),(float)kBtnW,(float)kBtnH); }

static void drawSymbol(SkCanvas* c, SkRect r, const char* sym, SkColor col, const SkFont& f) {
    SkPaint p; p.setAntiAlias(true); p.setColor(col);
    SkRect tb; f.measureText(sym,strlen(sym),SkTextEncoding::kUTF8,&tb);
    c->drawString(sym, r.centerX()-tb.width()/2.f-tb.left(), r.centerY()-tb.height()/2.f-tb.top(), f, p);
}

void skVideoView::Paint(SkCanvas* canvas) {
    const auto& th = skGetTheme();
    SkFont font(vvTf(), 11.f); font.setEdging(SkFont::Edging::kAntiAlias);
    SkFont small(vvTf(), 10.f); small.setEdging(SkFont::Edging::kAntiAlias);

    SkRRect outerRR; outerRR.setRectXY(SkRect::MakeXYWH((float)x,(float)y,(float)w,(float)h),6.f,6.f);
    SkPaint bgP; bgP.setAntiAlias(true); bgP.setColor(SkColorSetRGB(12,12,16));
    canvas->save(); canvas->clipRRect(outerRR, SkClipOp::kIntersect, true);
    canvas->drawRRect(outerRR, bgP);

    // Frame area
    SkRect fr = frameRect();
    { SkPaint fp; fp.setColor(SkColorSetRGB(8,8,12)); canvas->drawRect(fr,fp); }

    if (m_filename.empty()) {
        // No file loaded
        SkPaint tp; tp.setAntiAlias(true); tp.setColor(SkColorSetRGB(80,80,100));
        canvas->drawString("No video loaded", fr.centerX()-50.f, fr.centerY(), font, tp);
    } else {
        // Placeholder: show filename + state
        SkColor fc = (m_state==State::Playing) ? SkColorSetRGB(40,140,80) : SkColorSetRGB(60,80,120);
        { SkPaint fp2; fp2.setAntiAlias(true); fp2.setColor(fc);
          canvas->drawRect(SkRect::MakeXYWH(fr.left(),fr.top(),fr.width(),fr.height()*0.5f),fp2); }
        SkPaint tp; tp.setAntiAlias(true); tp.setColor(SkColorSetRGB(220,220,230));
        canvas->drawString(m_filename.c_str(), fr.left()+8.f, fr.centerY()+6.f, font, tp);
        if (m_state == State::Playing) {
            // Animated ring
            float ang = (float)(m_tickCount % 20) / 20.f * 360.f;
            SkPaint rp; rp.setAntiAlias(true); rp.setStyle(SkPaint::kStroke_Style);
            rp.setStrokeWidth(2.f); rp.setColor(SkColorSetRGB(255,255,255));
            canvas->drawArc(SkRect::MakeXYWH(fr.centerX()-12.f,fr.top()+8.f,24.f,24.f),ang,270.f,false,rp);
        }
    }

    // Control bar background
    SkRect ctrl = SkRect::MakeXYWH((float)x,(float)(y+h-kCtrlH),(float)w,(float)kCtrlH);
    { SkPaint cp; cp.setColor(SkColorSetRGB(20,20,26)); canvas->drawRect(ctrl,cp); }

    // Seek bar track
    SkRect sk = seekRect();
    { SkPaint trk; trk.setAntiAlias(true); trk.setColor(SkColorSetRGB(50,50,60));
      SkRRect srr; srr.setRectXY(sk,2.f,2.f); canvas->drawRRect(srr,trk); }
    if (m_duration > 0.f) {
        float ratio = m_position / m_duration;
        SkRect fill = SkRect::MakeXYWH(sk.left(),sk.top(),sk.width()*ratio,(float)kSeekH);
        SkPaint fp; fp.setAntiAlias(true); fp.setColor(th.accent);
        SkRRect frr; frr.setRectXY(fill,2.f,2.f); canvas->drawRRect(frr,fp);
        // Thumb
        float tx = sk.left()+sk.width()*ratio;
        SkPaint thumb; thumb.setAntiAlias(true); thumb.setColor(SK_ColorWHITE);
        canvas->drawCircle(tx,sk.centerY(),5.f,thumb);
    }

    // Buttons
    auto drawBtn = [&](SkRect r, const char* sym, bool hov) {
        SkRRect br; br.setRectXY(r,4.f,4.f);
        SkPaint bp; bp.setAntiAlias(true);
        bp.setColor(hov ? SkColorSetARGB(60,255,255,255) : SkColorSetARGB(30,255,255,255));
        canvas->drawRRect(br,bp);
        drawSymbol(canvas, r, sym, SK_ColorWHITE, font);
    };
    drawBtn(playBtn(),  "\xe2\x96\xb6", m_playHov);   // ▶
    drawBtn(pauseBtn(), "\xe2\x80\x96", m_pauseHov);   // ‖
    drawBtn(stopBtn(),  "\xe2\x96\xa0", m_stopHov);    // ■

    // Time display
    char timeBuf[32];
    int pos = (int)m_position, dur = (int)m_duration;
    snprintf(timeBuf,sizeof(timeBuf),"%d:%02d / %d:%02d", pos/60,pos%60,dur/60,dur%60);
    SkPaint tp2; tp2.setAntiAlias(true); tp2.setColor(SkColorSetRGB(160,160,180));
    canvas->drawString(timeBuf, stopBtn().right()+8.f, ctrl.centerY()+4.f, small, tp2);

    // Outer border
    SkPaint brd; brd.setAntiAlias(true); brd.setStyle(SkPaint::kStroke_Style);
    brd.setStrokeWidth(0.75f); brd.setColor(th.panelBorder);
    canvas->drawRRect(outerRR,brd);
    canvas->restore();
}

void skVideoView::OnEvent(const skEvent& ev) {
    if (ev.type == skEventType::MouseMove) {
        m_playHov  = playBtn().contains((float)ev.x,(float)ev.y);
        m_pauseHov = pauseBtn().contains((float)ev.x,(float)ev.y);
        m_stopHov  = stopBtn().contains((float)ev.x,(float)ev.y);
    }
    if (ev.type == skEventType::MouseDown) {
        if (playBtn().contains((float)ev.x,(float)ev.y))  { play();  return; }
        if (pauseBtn().contains((float)ev.x,(float)ev.y)) { pause(); return; }
        if (stopBtn().contains((float)ev.x,(float)ev.y))  { stop();  return; }
        // Seek click
        SkRect sk = seekRect();
        if (sk.contains((float)ev.x,(float)ev.y) && m_duration > 0.f) {
            float ratio = ((float)ev.x - sk.left()) / sk.width();
            seek(ratio * m_duration);
        }
    }
}
