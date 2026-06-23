#include "skSeparator.h"
#include "skTheme.h"
#include <include/core/SkPaint.h>

skSeparator::skSeparator(int sx, int sy, int sw, int sh)
    : skWidget(sx, sy, sw, sh) {}

std::shared_ptr<skSeparator> skSeparator::make(int sx, int sy, int sw, int sh) {
    return std::make_shared<skSeparator>(sx, sy, sw, sh);
}
std::shared_ptr<skSeparator> skSeparator::pos(int px, int py)  { x = px; y = py; return shared_this(); }
std::shared_ptr<skSeparator> skSeparator::size(int pw, int ph) { w = pw; h = ph; return shared_this(); }

void skSeparator::Paint(SkCanvas* canvas) {
    SkPaint p;
    p.setColor(skGetTheme().panelBorder);
    p.setAntiAlias(false);

    if (h > w) {
        float cx = (float)x + (float)w / 2.f;
        canvas->drawLine(cx, (float)y, cx, (float)(y + h), p);
    } else {
        float cy = (float)y + (float)h / 2.f;
        canvas->drawLine((float)x, cy, (float)(x + w), cy, p);
    }
}
