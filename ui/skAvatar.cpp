#include "skAvatar.h"
#include "skTheme.h"
#include "skTypeface.h"
#include <include/core/SkPaint.h>
#include <include/core/SkFont.h>

static const SkColor kPalette[] = {
    SkColorSetRGB( 55, 120, 220),
    SkColorSetRGB(150,  50, 210),
    SkColorSetRGB( 20, 155, 145),
    SkColorSetRGB(210,  80,  40),
    SkColorSetRGB( 40, 155,  55),
    SkColorSetRGB(200, 125,  15),
};

skAvatar::skAvatar(int cx, int cy, int radius)
    : skWidget(cx - radius, cy - radius, radius * 2, radius * 2), m_radius(radius) {}

SkColor skAvatar::pickColor() const {
    if (m_colorSet) return m_bg;
    if (m_initials.empty()) return kPalette[0];
    int idx = 0;
    for (unsigned char c : m_initials) idx = (idx * 31 + c) % 6;
    return kPalette[idx];
}

void skAvatar::Paint(SkCanvas* canvas) {
    float cx = (float)x + (float)m_radius;
    float cy = (float)y + (float)m_radius;
    float r  = (float)m_radius;

    SkPaint fill; fill.setAntiAlias(true); fill.setColor(pickColor());
    canvas->drawCircle(cx, cy, r, fill);

    if (!m_initials.empty()) {
        sk_sp<SkTypeface> tf = skGetSystemTypeface();
        SkFont font(tf, r * 0.72f);
        font.setEdging(SkFont::Edging::kAntiAlias);

        SkRect bounds;
        font.measureText(m_initials.c_str(), m_initials.size(), SkTextEncoding::kUTF8, &bounds);
        SkPaint tp; tp.setAntiAlias(true); tp.setColor(SK_ColorWHITE);
        canvas->drawString(m_initials.c_str(),
            cx - bounds.width() * 0.5f - bounds.left(),
            cy - bounds.height() * 0.5f - bounds.top(),
            font, tp);
    }

    SkPaint ring; ring.setAntiAlias(true);
    ring.setStyle(SkPaint::kStroke_Style);
    ring.setStrokeWidth(1.5f);
    ring.setColor(SkColorSetARGB(40, 0, 0, 0));
    canvas->drawCircle(cx, cy, r - 0.75f, ring);
}
