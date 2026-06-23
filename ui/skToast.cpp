#include "skToast.h"
#include "skTypeface.h"
#include "skTheme.h"
#include <include/core/SkPaint.h>
#include <include/core/SkFont.h>
#include <include/core/SkRRect.h>
#include <algorithm>

skToast::skToast(int tx, int ty, int tw, int th)
    : skWidget(tx, ty, tw, th) {}

std::shared_ptr<skToast> skToast::make(int tx, int ty, int tw, int th) {
    return std::make_shared<skToast>(tx, ty, tw, th);
}
std::shared_ptr<skToast> skToast::pos(int px, int py)  { x = px; y = py; return shared_this(); }
std::shared_ptr<skToast> skToast::size(int pw, int ph) { w = pw; h = ph; return shared_this(); }

void skToast::show(const std::string& msg, int durationTicks) {
    m_msg         = msg;
    m_countdown   = durationTicks;
    m_slideOffset = (float)(h + 20); // start fully below rest position
    m_state       = State::SlideIn;
    setVisible(true);
}

void skToast::onTick() {
    if (!visible()) return;
    const float step = (float)(h + 20) / 5.f; // slide in/out over 5 ticks (500 ms)

    switch (m_state) {
        case State::SlideIn:
            m_slideOffset = std::max(0.f, m_slideOffset - step);
            if (m_slideOffset <= 0.f) m_state = State::Hold;
            break;
        case State::Hold:
            if (--m_countdown <= 0) m_state = State::SlideOut;
            break;
        case State::SlideOut:
            m_slideOffset = std::min((float)(h + 20), m_slideOffset + step);
            if (m_slideOffset >= (float)(h + 20)) {
                setVisible(false);
                m_state = State::Idle;
            }
            break;
        default: break;
    }
}

void skToast::Paint(SkCanvas* canvas) {
    if (!visible()) return;
    const auto& th = skGetTheme();

    float drawY = (float)y + m_slideOffset;
    SkRect r = SkRect::MakeXYWH((float)x, drawY, (float)w, (float)h);

    // Drop shadow
    SkPaint shadow;
    shadow.setAntiAlias(true);
    shadow.setColor(SkColorSetARGB(35, 0, 0, 0));
    SkRRect shadowRR;
    shadowRR.setRectXY(r.makeOffset(0.f, 3.f), 10.f, 10.f);
    canvas->drawRRect(shadowRR, shadow);

    // Background
    SkPaint bg;
    bg.setAntiAlias(true);
    bg.setColor(th.panelBg);
    SkRRect rr;
    rr.setRectXY(r, 10.f, 10.f);
    canvas->drawRRect(rr, bg);

    // Accent left stripe
    canvas->save();
    canvas->clipRRect(rr, SkClipOp::kIntersect, true);
    SkPaint stripe;
    stripe.setColor(th.accent);
    canvas->drawRect(SkRect::MakeXYWH((float)x, drawY, 4.f, (float)h), stripe);
    canvas->restore();

    // Border
    SkPaint brd;
    brd.setAntiAlias(true);
    brd.setStyle(SkPaint::kStroke_Style);
    brd.setStrokeWidth(1.f);
    brd.setColor(th.panelBorder);
    canvas->drawRRect(rr, brd);

    // Message text
    static sk_sp<SkTypeface> s_tf = skGetSystemTypeface();
    SkFont font(s_tf, 13.f);
    font.setEdging(SkFont::Edging::kAntiAlias);

    SkRect tb;
    font.measureText(m_msg.c_str(), m_msg.size(), SkTextEncoding::kUTF8, &tb);
    float tx2 = (float)x + 16.f - tb.left();
    float ty2 = drawY + ((float)h - tb.height()) / 2.f - tb.top();

    SkPaint tp;
    tp.setAntiAlias(true);
    tp.setColor(th.textPrimary);
    canvas->drawString(m_msg.c_str(), tx2, ty2, font, tp);
}
