#include "skButton.h"
#include "skTypeface.h"
#include "skTheme.h"
#include <include/core/SkPaint.h>
#include <include/core/SkRRect.h>
#include <include/core/SkFont.h>
#include <algorithm>

skButton::skButton(int bx, int by, int bw, int bh, std::string label)
    : skWidget(bx, by, bw, bh), m_label(std::move(label)) {}

std::shared_ptr<skButton> skButton::make(int bx, int by, int bw, int bh, std::string label) {
    return std::make_shared<skButton>(bx, by, bw, bh, std::move(label));
}

std::shared_ptr<skButton> skButton::label(std::string lbl) {
    m_label = std::move(lbl);
    return shared_this();
}

std::shared_ptr<skButton> skButton::onClick(std::function<void()> cb) {
    m_onClick = std::move(cb);
    return shared_this();
}

std::shared_ptr<skButton> skButton::pos(int px, int py) {
    x = px; y = py;
    return shared_this();
}

std::shared_ptr<skButton> skButton::size(int pw, int ph) {
    w = pw; h = ph;
    return shared_this();
}

void skButton::Paint(SkCanvas* canvas) {
    canvas->save();
    const auto& th = skGetTheme();

    SkPaint fill;
    fill.setAntiAlias(true);
    fill.setStyle(SkPaint::kFill_Style);
    fill.setColor(m_pressed ? th.accentPress
                            : skLerpColor(th.accent, th.accentHover, m_hoverAnim));

    SkRRect rr;
    rr.setRectXY(SkRect::MakeXYWH((float)x, (float)y, (float)w, (float)h), 6.f, 6.f);
    canvas->drawRRect(rr, fill);

    SkPaint border;
    border.setAntiAlias(true);
    border.setStyle(SkPaint::kStroke_Style);
    border.setStrokeWidth(1.f);
    border.setColor(SkColorSetARGB(60, 0, 0, 0));
    canvas->drawRRect(rr, border);

    static sk_sp<SkTypeface> s_tf = skGetSystemTypeface();
    SkFont font(s_tf, 15.f);
    font.setEdging(SkFont::Edging::kAntiAlias);

    SkRect bounds;
    font.measureText(m_label.c_str(), m_label.size(), SkTextEncoding::kUTF8, &bounds);
    float tx = x + (w - bounds.width())  / 2.f - bounds.left();
    float ty = y + (h - bounds.height()) / 2.f - bounds.top();

    SkPaint text;
    text.setAntiAlias(true);
    text.setColor(th.textOnAccent);
    canvas->drawString(m_label.c_str(), tx, ty, font, text);

    canvas->restore();
}

void skButton::onTick() {
    float target = m_hovered ? 1.f : 0.f;
    if (m_hoverAnim < target) m_hoverAnim = std::min(1.f, m_hoverAnim + 0.25f);
    else                      m_hoverAnim = std::max(0.f, m_hoverAnim - 0.25f);
}

void skButton::OnEvent(const skEvent& event) {
    switch (event.type) {
        case skEventType::MouseMove:  m_hovered = contains(event.x, event.y); break;
        case skEventType::MouseDown:  if (contains(event.x, event.y)) m_pressed = true; break;
        case skEventType::MouseUp:
            if (m_pressed && contains(event.x, event.y) && m_onClick) m_onClick();
            m_pressed = false;
            break;
        default: break;
    }
}
