#include "skProgressBar.h"
#include <include/core/SkPaint.h>
#include <include/core/SkRRect.h>

skProgressBar::skProgressBar(int px, int py, int pw, int ph) : skWidget(px, py, pw, ph) {}

void skProgressBar::Paint(SkCanvas* canvas) {
    canvas->save();

    const float r = (float)h / 2.f;
    SkRect rect = SkRect::MakeXYWH((float)x, (float)y, (float)w, (float)h);

    // Track
    SkPaint track;
    track.setAntiAlias(true);
    track.setColor(SkColorSetRGB(218, 220, 228));
    SkRRect trackRR;
    trackRR.setRectXY(rect, r, r);
    canvas->drawRRect(trackRR, track);

    // Fill
    float fillW = (float)w * m_value;
    if (fillW > 0.f) {
        SkPaint fill;
        fill.setAntiAlias(true);
        fill.setColor(SkColorSetRGB(55, 120, 220));

        // Clip to track bounds so the fill corners don't poke out
        canvas->save();
        canvas->clipRRect(trackRR, true);
        SkRRect fillRR;
        fillRR.setRectXY(SkRect::MakeXYWH((float)x, (float)y, fillW, (float)h), r, r);
        canvas->drawRRect(fillRR, fill);
        canvas->restore();
    }

    canvas->restore();
}
