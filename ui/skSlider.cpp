#include "skSlider.h"
#include <include/core/SkPaint.h>
#include <include/core/SkRRect.h>
#include <algorithm>

skSlider::skSlider(int sx, int sy, int sw, int sh, float min, float max)
    : skWidget(sx, sy, sw, sh), m_min(min), m_max(max),
      m_value(min + (max - min) * 0.5f) {}

void skSlider::setValue(float v) {
    m_value = std::max(m_min, std::min(m_max, v));
}

float skSlider::thumbCx() const {
    float ratio = (m_value - m_min) / (m_max - m_min);
    return (float)x + kThumbR + ratio * ((float)w - 2.f * kThumbR);
}

void skSlider::updateFromX(int mouseX) {
    float usable = (float)w - 2.f * kThumbR;
    float ratio  = ((float)mouseX - (float)x - kThumbR) / usable;
    ratio = std::max(0.f, std::min(1.f, ratio));
    float newVal = m_min + ratio * (m_max - m_min);
    if (newVal != m_value) {
        m_value = newVal;
        if (m_onChange) m_onChange(m_value);
    }
}

void skSlider::Paint(SkCanvas* canvas) {
    canvas->save();

    float cy      = (float)y + (float)h / 2.f;
    float trackX0 = (float)x + kThumbR;
    float trackX1 = (float)x + (float)w - kThumbR;
    float cx      = thumbCx();

    // Track background
    SkPaint track;
    track.setAntiAlias(true);
    track.setColor(SkColorSetRGB(210, 210, 210));
    SkRRect trackRR;
    trackRR.setRectXY(SkRect::MakeLTRB(trackX0, cy - kTrackH / 2.f,
                                        trackX1, cy + kTrackH / 2.f),
                      kTrackH / 2.f, kTrackH / 2.f);
    canvas->drawRRect(trackRR, track);

    // Filled portion
    if (cx > trackX0) {
        SkPaint filled;
        filled.setAntiAlias(true);
        filled.setColor(SkColorSetRGB(55, 120, 220));
        SkRRect fillRR;
        fillRR.setRectXY(SkRect::MakeLTRB(trackX0, cy - kTrackH / 2.f,
                                           cx,      cy + kTrackH / 2.f),
                         kTrackH / 2.f, kTrackH / 2.f);
        canvas->drawRRect(fillRR, filled);
    }

    // Thumb
    SkPaint thumb;
    thumb.setAntiAlias(true);
    thumb.setColor(m_dragging ? SkColorSetRGB(20, 80, 180)
                              : SkColorSetRGB(55, 120, 220));
    canvas->drawCircle(cx, cy, kThumbR, thumb);

    // Thumb ring
    SkPaint ring;
    ring.setAntiAlias(true);
    ring.setStyle(SkPaint::kStroke_Style);
    ring.setStrokeWidth(1.5f);
    ring.setColor(SkColorSetARGB(60, 0, 0, 0));
    canvas->drawCircle(cx, cy, kThumbR, ring);

    canvas->restore();
}

void skSlider::OnEvent(const skEvent& event) {
    switch (event.type) {
        case skEventType::MouseDown:
            if (contains(event.x, event.y)) {
                m_dragging = true;
                updateFromX(event.x);
            }
            break;
        case skEventType::MouseMove:
            if (m_dragging) updateFromX(event.x);
            break;
        case skEventType::MouseUp:
            m_dragging = false;
            break;
    }
}
