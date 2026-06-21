#include "skButton.h"
#include "skTypeface.h"
#include <include/core/SkPaint.h>
#include <include/core/SkRRect.h>
#include <include/core/SkFont.h>

skButton::skButton(int bx, int by, int bw, int bh, std::string label)
    : skWidget(bx, by, bw, bh), m_label(std::move(label)) {}

void skButton::Paint(SkCanvas* canvas) {
    canvas->save();

    // Background
    SkPaint fill;
    fill.setAntiAlias(true);
    fill.setStyle(SkPaint::kFill_Style);
    if (m_pressed)
        fill.setColor(SkColorSetRGB(20, 80, 180));
    else if (m_hovered)
        fill.setColor(SkColorSetRGB(80, 150, 255));
    else
        fill.setColor(SkColorSetRGB(55, 120, 220));

    SkRRect rrect;
    rrect.setRectXY(SkRect::MakeXYWH((float)x, (float)y, (float)w, (float)h), 6.0f, 6.0f);
    canvas->drawRRect(rrect, fill);

    // Border
    SkPaint border;
    border.setAntiAlias(true);
    border.setStyle(SkPaint::kStroke_Style);
    border.setStrokeWidth(1.0f);
    border.setColor(SkColorSetARGB(80, 0, 0, 0));
    canvas->drawRRect(rrect, border);

    // Label — load typeface once per process
    static sk_sp<SkTypeface> s_tf = skGetSystemTypeface();
    SkFont font(s_tf, 20.0f);
    font.setEdging(SkFont::Edging::kAntiAlias);

    SkRect bounds;
    font.measureText(m_label.c_str(), m_label.size(), SkTextEncoding::kUTF8, &bounds);

    float tx = x + (w - bounds.width())  / 2.0f - bounds.left();
    float ty = y + (h - bounds.height()) / 2.0f - bounds.top();

    SkPaint text;
    text.setAntiAlias(true);
    text.setColor(SK_ColorWHITE);
    canvas->drawString(m_label.c_str(), tx, ty, font, text);

    canvas->restore();
}

void skButton::OnEvent(const skEvent& event) {
    switch (event.type) {
        case skEventType::MouseMove:
            m_hovered = contains(event.x, event.y);
            break;
        case skEventType::MouseDown:
            if (contains(event.x, event.y))
                m_pressed = true;
            break;
        case skEventType::MouseUp:
            if (m_pressed && contains(event.x, event.y) && m_onClick)
                m_onClick();
            m_pressed = false;
            break;
    }
}
