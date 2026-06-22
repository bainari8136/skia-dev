#include "skTooltip.h"
#include "skTheme.h"
#include "skTypeface.h"
#include <include/core/SkPaint.h>
#include <include/core/SkFont.h>
#include <include/core/SkCanvas.h>
#include <include/core/SkRRect.h>
#include <algorithm>

static sk_sp<SkTypeface>& ttTf() {
    static sk_sp<SkTypeface> tf = skGetSystemTypeface();
    return tf;
}

skTooltip::skTooltip(int winW, int winH)
    : skWidget(0, 0, winW, winH) {
    setVisible(false);
}

void skTooltip::showAt(int anchorX, int anchorY, const std::string& text, int durationTicks) {
    m_anchorX   = anchorX;
    m_anchorY   = anchorY;
    m_text      = text;
    m_maxTicks  = durationTicks;
    m_ticksLeft = durationTicks;
    m_pinned    = (durationTicks == 0);
    setVisible(true);
}

void skTooltip::hide() {
    setVisible(false);
}

void skTooltip::onTick() {
    if (!visible() || m_pinned) return;
    if (m_ticksLeft > 0) {
        --m_ticksLeft;
        if (m_ticksLeft == 0) setVisible(false);
    }
}

void skTooltip::Paint(SkCanvas* canvas) {
    if (!visible()) return;
    const auto& th = skGetTheme();

    SkFont font(ttTf(), 11.f); font.setEdging(SkFont::Edging::kAntiAlias);
    SkRect tb; font.measureText(m_text.c_str(), m_text.size(), SkTextEncoding::kUTF8, &tb);
    float bw = tb.width()  + 2*kPadX;
    float bh = tb.height() + 2*kPadY;

    // Keep within window bounds
    float bx = (float)m_anchorX;
    float by = (float)m_anchorY + kOffset;
    if (bx + bw > (float)w - 4.f) bx = (float)w - bw - 4.f;
    if (by + bh > (float)h - 4.f) by = (float)m_anchorY - bh - 4.f;
    bx = std::max(4.f, bx);

    // Shadow
    SkPaint shP; shP.setAntiAlias(true);
    shP.setColor(SkColorSetARGB(40,0,0,0));
    SkRRect shRR; shRR.setRectXY(SkRect::MakeXYWH(bx+2.f,by+2.f,bw,bh),kR,kR);
    canvas->drawRRect(shRR, shP);

    // Box
    SkRRect rr; rr.setRectXY(SkRect::MakeXYWH(bx,by,bw,bh),kR,kR);
    SkPaint bgP; bgP.setAntiAlias(true); bgP.setColor(th.panelBg);
    canvas->drawRRect(rr, bgP);
    SkPaint brd; brd.setAntiAlias(true); brd.setStyle(SkPaint::kStroke_Style);
    brd.setStrokeWidth(0.75f); brd.setColor(th.panelBorder);
    canvas->drawRRect(rr, brd);

    // Text
    SkPaint tp; tp.setAntiAlias(true); tp.setColor(th.textPrimary);
    canvas->drawString(m_text.c_str(), bx+kPadX, by+kPadY-tb.top(), font, tp);
}

bool skTooltip::handleEvent(const skEvent& ev) {
    if (!visible()) return false;
    // Dismiss on click anywhere
    if (ev.type == skEventType::MouseDown && !m_pinned) {
        setVisible(false); return false;
    }
    return false; // never consume events — tooltip is passive
}
