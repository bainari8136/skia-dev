#include "skRadioButton.h"
#include "skTypeface.h"
#include "skTheme.h"
#include <include/core/SkPaint.h>
#include <include/core/SkFont.h>

// ---- skRadioGroup ----

void skRadioGroup::add(skRadioButton* btn) {
    m_buttons.push_back(btn);
}

void skRadioGroup::select(skRadioButton* btn) {
    for (auto* b : m_buttons) b->setSelected(false);
    m_selected = btn;
    if (btn) btn->setSelected(true);
}

// ---- skRadioButton ----

skRadioButton::skRadioButton(int rx, int ry, int rw, int rh, std::string label, skRadioGroup* group)
    : skWidget(rx, ry, rw, rh), m_label(std::move(label)), m_group(group) {
    if (group) group->add(this);
}

void skRadioButton::Paint(SkCanvas* canvas) {
    canvas->save();
    const auto& th = skGetTheme();

    const float r    = (float)(h - 6) / 2.f;
    const float cx   = (float)x + 3.f + r;
    const float cy   = (float)y + (float)h / 2.f;

    // Outer ring
    SkPaint ring;
    ring.setAntiAlias(true);
    ring.setStyle(SkPaint::kStroke_Style);
    ring.setStrokeWidth(1.5f);
    ring.setColor(m_selected ? th.accent : th.inputBorder);
    canvas->drawCircle(cx, cy, r, ring);

    // Background fill (hover effect)
    if (m_hovered) {
        SkPaint hov;
        hov.setAntiAlias(true);
        hov.setColor(SkColorSetARGB(20,
            SkColorGetR(th.accent),
            SkColorGetG(th.accent),
            SkColorGetB(th.accent)));
        canvas->drawCircle(cx, cy, r, hov);
    }

    // Inner dot when selected
    if (m_selected) {
        SkPaint dot;
        dot.setAntiAlias(true);
        dot.setColor(th.accent);
        canvas->drawCircle(cx, cy, r * 0.5f, dot);
    }

    // Label
    static sk_sp<SkTypeface> s_tf = skGetSystemTypeface();
    SkFont font(s_tf, 15.f);
    font.setEdging(SkFont::Edging::kAntiAlias);

    SkRect bounds;
    font.measureText(m_label.c_str(), m_label.size(), SkTextEncoding::kUTF8, &bounds);
    float tx = cx + r + 8.f - bounds.left();
    float ty = (float)y + ((float)h - bounds.height()) / 2.f - bounds.top();

    SkPaint text;
    text.setAntiAlias(true);
    text.setColor(th.textPrimary);
    canvas->drawString(m_label.c_str(), tx, ty, font, text);

    canvas->restore();
}

void skRadioButton::OnEvent(const skEvent& event) {
    switch (event.type) {
        case skEventType::MouseMove:
            m_hovered = contains(event.x, event.y);
            break;
        case skEventType::MouseUp:
            if (contains(event.x, event.y) && !m_selected) {
                if (m_group) m_group->select(this);
                else         m_selected = true;
                if (m_onSelect) m_onSelect();
            }
            break;
        default: break;
    }
}
