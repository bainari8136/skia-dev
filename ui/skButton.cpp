#include "skButton.h"
#include "skTypeface.h"
#include "skTheme.h"
#include <include/core/SkPaint.h>
#include <include/core/SkRRect.h>
#include <include/core/SkFont.h>

skButton::skButton(int bx, int by, int bw, int bh, std::string label)
    : skWidget(bx, by, bw, bh), m_label(std::move(label)) {}

void skButton::Paint(SkCanvas* canvas) {
    canvas->save();
    const auto& th = skGetTheme();

    SkPaint fill;
    fill.setAntiAlias(true);
    fill.setStyle(SkPaint::kFill_Style);
    fill.setColor(m_pressed ? th.accentPress
                : m_hovered ? th.accentHover
                            : th.accent);

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
