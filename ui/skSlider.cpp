#include "skSlider.h"
#include "skTheme.h"
#include <include/core/SkPaint.h>
#include <include/core/SkRRect.h>
#include <algorithm>

skSlider::skSlider(int sx, int sy, int sw, int sh, float min, float max)
    : skWidget(sx, sy, sw, sh), m_min(min), m_max(max),
      m_value(min + (max - min) * 0.5f) {}

void skSlider::setValue(float v) { m_value = std::max(m_min, std::min(m_max, v)); }

float skSlider::thumbCx() const {
    float ratio = (m_value - m_min) / (m_max - m_min);
    return (float)x + kThumbR + ratio * ((float)w - 2.f * kThumbR);
}

void skSlider::updateFromX(int mouseX) {
    float ratio  = ((float)mouseX - (float)x - kThumbR) / ((float)w - 2.f * kThumbR);
    ratio = std::max(0.f, std::min(1.f, ratio));
    float newVal = m_min + ratio * (m_max - m_min);
    if (newVal != m_value) { m_value = newVal; if (m_onChange) m_onChange(m_value); }
}

void skSlider::Paint(SkCanvas* canvas) {
    canvas->save();
    const auto& th = skGetTheme();

    float cy      = (float)y + (float)h / 2.f;
    float trackX0 = (float)x + kThumbR;
    float trackX1 = (float)x + (float)w - kThumbR;
    float cx      = thumbCx();

    SkPaint track;
    track.setAntiAlias(true);
    track.setColor(th.trackBg);
    SkRRect tRR;
    tRR.setRectXY(SkRect::MakeLTRB(trackX0, cy - kTrackH/2.f, trackX1, cy + kTrackH/2.f),
                  kTrackH/2.f, kTrackH/2.f);
    canvas->drawRRect(tRR, track);

    if (cx > trackX0) {
        SkPaint filled;
        filled.setAntiAlias(true);
        filled.setColor(th.accent);
        SkRRect fRR;
        fRR.setRectXY(SkRect::MakeLTRB(trackX0, cy - kTrackH/2.f, cx, cy + kTrackH/2.f),
                      kTrackH/2.f, kTrackH/2.f);
        canvas->drawRRect(fRR, filled);
    }

    SkPaint thumb;
    thumb.setAntiAlias(true);
    thumb.setColor(m_dragging ? th.accentPress : th.accent);
    canvas->drawCircle(cx, cy, kThumbR, thumb);

    SkPaint ring;
    ring.setAntiAlias(true);
    ring.setStyle(SkPaint::kStroke_Style);
    ring.setStrokeWidth(1.5f);
    ring.setColor(SkColorSetARGB(50, 0, 0, 0));
    canvas->drawCircle(cx, cy, kThumbR, ring);

    canvas->restore();
}

void skSlider::OnEvent(const skEvent& event) {
    switch (event.type) {
        case skEventType::MouseDown:
            if (contains(event.x, event.y)) { m_dragging = true; updateFromX(event.x); }
            break;
        case skEventType::MouseMove:
            if (m_dragging) updateFromX(event.x);
            break;
        case skEventType::MouseUp:
            m_dragging = false;
            break;
        default: break;
    }
}

std::shared_ptr<skSlider> skSlider::make(int sx, int sy, int sw, int sh, float min, float max) {
    return std::make_shared<skSlider>(sx, sy, sw, sh, min, max);
}
std::shared_ptr<skSlider> skSlider::withValue(float v)                         { setValue(v); return shared_this(); }
std::shared_ptr<skSlider> skSlider::onChange(std::function<void(float)> cb)    { m_onChange = std::move(cb); return shared_this(); }
std::shared_ptr<skSlider> skSlider::pos(int px, int py)                        { x = px; y = py; return shared_this(); }
std::shared_ptr<skSlider> skSlider::size(int pw, int ph)                       { w = pw; h = ph; return shared_this(); }
