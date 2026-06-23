#include "skToggle.h"
#include "skTypeface.h"
#include "skTheme.h"
#include <include/core/SkPaint.h>
#include <include/core/SkFont.h>
#include <include/core/SkRRect.h>
#include <algorithm>

skToggle::skToggle(int tx, int ty, int tw, int th, std::string label)
    : skWidget(tx, ty, tw, th), m_label(std::move(label)) {}

void skToggle::onTick() {
    float target = m_checked ? 1.f : 0.f;
    if (m_thumbAnim == target) return; // already settled — nothing to do
    if (m_thumbAnim < target) m_thumbAnim = std::min(1.f, m_thumbAnim + 0.4f);
    else                      m_thumbAnim = std::max(0.f, m_thumbAnim - 0.4f);
}

void skToggle::Paint(SkCanvas* canvas) {
    const auto& th = skGetTheme();

    // Pill centered vertically
    float pillY = (float)y + ((float)h - kPillH) / 2.f;
    SkRect pillR = SkRect::MakeXYWH((float)x, pillY, (float)kPillW, (float)kPillH);

    // Background: lerp between trackBg (off) and accent (on)
    SkColor bgColor = skLerpColor(th.trackBg, th.accent, m_thumbAnim);
    SkPaint bg;
    bg.setAntiAlias(true);
    bg.setColor(bgColor);
    SkRRect pill;
    pill.setRectXY(pillR, (float)kPillH / 2.f, (float)kPillH / 2.f);
    canvas->drawRRect(pill, bg);

    // Hover ring
    if (m_hovered) {
        SkPaint ring;
        ring.setAntiAlias(true);
        ring.setStyle(SkPaint::kStroke_Style);
        ring.setStrokeWidth(2.f);
        ring.setColor(SkColorSetARGB(50,
            SkColorGetR(th.accent), SkColorGetG(th.accent), SkColorGetB(th.accent)));
        canvas->drawRRect(pill, ring);
    }

    // Thumb
    const float thumbR = (float)(kPillH - 6) / 2.f;
    float offCx = (float)x + 3.f + thumbR;
    float onCx  = (float)x + (float)kPillW - 3.f - thumbR;
    float thumbCx = offCx + (onCx - offCx) * m_thumbAnim;
    float thumbCy = pillY + (float)kPillH / 2.f;

    // Drop shadow so the white thumb is visible on light backgrounds
    SkPaint shadow;
    shadow.setAntiAlias(true);
    shadow.setColor(SkColorSetARGB(60, 0, 0, 0));
    canvas->drawCircle(thumbCx + 0.5f, thumbCy + 1.f, thumbR, shadow);

    SkPaint thumb;
    thumb.setAntiAlias(true);
    thumb.setColor(SK_ColorWHITE);
    canvas->drawCircle(thumbCx, thumbCy, thumbR, thumb);

    // Optional label to the right of the pill
    if (!m_label.empty()) {
        static sk_sp<SkTypeface> s_tf = skGetSystemTypeface();
        SkFont font(s_tf, 14.f);
        font.setEdging(SkFont::Edging::kAntiAlias);

        SkRect lb;
        font.measureText(m_label.c_str(), m_label.size(), SkTextEncoding::kUTF8, &lb);
        float lx = (float)x + kPillW + 10.f - lb.left();
        float ly = (float)y + ((float)h - lb.height()) / 2.f - lb.top();

        SkPaint tp;
        tp.setAntiAlias(true);
        tp.setColor(th.textPrimary);
        canvas->drawString(m_label.c_str(), lx, ly, font, tp);
    }
}

void skToggle::OnEvent(const skEvent& event) {
    switch (event.type) {
        case skEventType::MouseMove:
            m_hovered = contains(event.x, event.y);
            break;
        case skEventType::MouseDown:
            if (contains(event.x, event.y)) {
                m_checked = !m_checked;
                if (m_onChange) m_onChange(m_checked);
            }
            break;
        default: break;
    }
}
