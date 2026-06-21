#include "skPanel.h"
#include <include/core/SkPaint.h>
#include <include/core/SkRRect.h>

skPanel::skPanel(int px, int py, int pw, int ph) : skWidget(px, py, pw, ph) {}

void skPanel::Paint(SkCanvas* canvas) {
    canvas->save();

    SkRRect rr;
    rr.setRectXY(SkRect::MakeXYWH((float)x, (float)y, (float)w, (float)h),
                 m_radius, m_radius);

    // Drop shadow (draw a slightly offset darker rect first)
    SkPaint shadow;
    shadow.setAntiAlias(true);
    shadow.setColor(SkColorSetARGB(25, 0, 0, 0));
    SkRRect shadowRR;
    shadowRR.setRectXY(
        SkRect::MakeXYWH((float)x + 2.f, (float)y + 3.f, (float)w, (float)h),
        m_radius, m_radius);
    canvas->drawRRect(shadowRR, shadow);

    SkPaint fill;
    fill.setAntiAlias(true);
    fill.setColor(m_bg);
    canvas->drawRRect(rr, fill);

    SkPaint border;
    border.setAntiAlias(true);
    border.setStyle(SkPaint::kStroke_Style);
    border.setStrokeWidth(1.f);
    border.setColor(m_border);
    canvas->drawRRect(rr, border);

    canvas->restore();
}
