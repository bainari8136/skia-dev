#include "skCard.h"
#include "skTheme.h"
#include <include/core/SkPaint.h>
#include <include/core/SkRRect.h>

skCard::skCard(int cx, int cy, int cw, int ch, float radius)
    : skWidget(cx, cy, cw, ch), m_radius(radius) {}

void skCard::Paint(SkCanvas* canvas) {
    const auto& th = skGetTheme();

    // Multi-layer soft shadow (stacked translucent rects with increasing offset)
    static const struct { float dx, dy; uint8_t alpha; } kLayers[] = {
        {0, 1, 14}, {0, 2, 12}, {1, 3, 9},
        {1, 4,  7}, {2, 5,  5}, {2, 6, 3},
    };
    for (auto& l : kLayers) {
        SkRRect sRR;
        sRR.setRectXY(
            SkRect::MakeXYWH((float)x + l.dx * (m_elevation / 4.f),
                             (float)y + l.dy * (m_elevation / 4.f),
                             (float)w, (float)h),
            m_radius, m_radius);
        SkPaint sp;
        sp.setAntiAlias(true);
        sp.setColor(SkColorSetARGB(l.alpha, 0, 0, 0));
        canvas->drawRRect(sRR, sp);
    }

    // Card face
    SkRRect rr;
    rr.setRectXY(SkRect::MakeXYWH((float)x, (float)y, (float)w, (float)h), m_radius, m_radius);

    SkPaint fill;
    fill.setAntiAlias(true);
    fill.setColor(th.panelBg);
    canvas->drawRRect(rr, fill);

    SkPaint brd;
    brd.setAntiAlias(true);
    brd.setStyle(SkPaint::kStroke_Style);
    brd.setStrokeWidth(0.75f);
    brd.setColor(th.panelBorder);
    canvas->drawRRect(rr, brd);
}
