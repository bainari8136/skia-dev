#include "skProgressBar.h"
#include "skTheme.h"
#include <include/core/SkPaint.h>
#include <include/core/SkRRect.h>

skProgressBar::skProgressBar(int px, int py, int pw, int ph) : skWidget(px, py, pw, ph) {}

std::shared_ptr<skProgressBar> skProgressBar::make(int px, int py, int pw, int ph) {
    return std::make_shared<skProgressBar>(px, py, pw, ph);
}
std::shared_ptr<skProgressBar> skProgressBar::withValue(float v)  { setValue(v); return shared_this(); }
std::shared_ptr<skProgressBar> skProgressBar::pos(int px, int py) { x = px; y = py; return shared_this(); }
std::shared_ptr<skProgressBar> skProgressBar::size(int pw, int ph){ w = pw; h = ph; return shared_this(); }

void skProgressBar::Paint(SkCanvas* canvas) {
    canvas->save();
    const auto& th = skGetTheme();
    const float r  = (float)h / 2.f;
    SkRect rect = SkRect::MakeXYWH((float)x, (float)y, (float)w, (float)h);

    SkPaint track;
    track.setAntiAlias(true);
    track.setColor(th.trackBg);
    SkRRect trackRR;
    trackRR.setRectXY(rect, r, r);
    canvas->drawRRect(trackRR, track);

    if (m_value > 0.f) {
        SkPaint fill;
        fill.setAntiAlias(true);
        fill.setColor(th.accent);
        canvas->save();
        canvas->clipRRect(trackRR, true);
        SkRRect fillRR;
        fillRR.setRectXY(SkRect::MakeXYWH((float)x, (float)y, (float)w * m_value, (float)h),
                         r, r);
        canvas->drawRRect(fillRR, fill);
        canvas->restore();
    }

    canvas->restore();
}
