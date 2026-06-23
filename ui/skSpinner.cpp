#include "skSpinner.h"
#include "skTheme.h"
#include <include/core/SkPaint.h>

skSpinner::skSpinner(int sx, int sy, int size)
    : skWidget(sx, sy, size, size) {}

std::shared_ptr<skSpinner> skSpinner::make(int sx, int sy, int size) {
    return std::make_shared<skSpinner>(sx, sy, size);
}
std::shared_ptr<skSpinner> skSpinner::pos(int px, int py) { x = px; y = py; return shared_this(); }

void skSpinner::onTick() {
    if (!m_running) return;
    m_angle = fmodf(m_angle + 36.f, 360.f);
    if (m_autoStopTicks > 0 && --m_autoStopTicks == 0) m_running = false;
}

void skSpinner::Paint(SkCanvas* canvas) {
    if (!m_running) return;
    const auto& th = skGetTheme();

    float cx = (float)x + (float)w / 2.f;
    float cy = (float)y + (float)h / 2.f;
    float r  = (float)w / 2.f - 3.f;

    // Track ring
    SkPaint track;
    track.setAntiAlias(true);
    track.setStyle(SkPaint::kStroke_Style);
    track.setStrokeWidth(3.f);
    track.setColor(th.trackBg);
    canvas->drawCircle(cx, cy, r, track);

    // Spinning sweep arc (270° tail behind the head)
    SkPaint arc;
    arc.setAntiAlias(true);
    arc.setStyle(SkPaint::kStroke_Style);
    arc.setStrokeWidth(3.f);
    arc.setStrokeCap(SkPaint::kRound_Cap);
    arc.setColor(th.accent);

    SkRect oval = SkRect::MakeLTRB(cx - r, cy - r, cx + r, cy + r);
    canvas->drawArc(oval, m_angle - 270.f, 270.f, false, arc);
}
