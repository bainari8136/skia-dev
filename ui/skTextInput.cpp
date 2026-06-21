#include <Windows.h>
#include "skTextInput.h"
#include "skTypeface.h"
#include <include/core/SkPaint.h>
#include <include/core/SkFont.h>
#include <include/core/SkRRect.h>

skTextInput::skTextInput(int ix, int iy, int iw, int ih, std::string placeholder)
    : skWidget(ix, iy, iw, ih), m_placeholder(std::move(placeholder)) {}

void skTextInput::Paint(SkCanvas* canvas) {
    canvas->save();

    // Background
    SkPaint bg;
    bg.setAntiAlias(true);
    bg.setColor(SK_ColorWHITE);
    SkRRect rr;
    rr.setRectXY(SkRect::MakeXYWH((float)x, (float)y, (float)w, (float)h), 6.f, 6.f);
    canvas->drawRRect(rr, bg);

    // Border
    SkPaint border;
    border.setAntiAlias(true);
    border.setStyle(SkPaint::kStroke_Style);
    border.setStrokeWidth(m_focused ? 2.f : 1.f);
    border.setColor(m_focused ? SkColorSetRGB(55, 120, 220)
                              : SkColorSetRGB(190, 192, 200));
    canvas->drawRRect(rr, border);

    const float pad    = 10.f;
    const float textL  = (float)x + pad;
    const float textR  = (float)x + (float)w - pad;

    SkFont font(skGetSystemTypeface(), 16.f);
    font.setEdging(SkFont::Edging::kAntiAlias);

    // Clip text to field interior
    canvas->save();
    canvas->clipRect(SkRect::MakeLTRB(textL, (float)y + 2, textR, (float)y + (float)h - 2));

    const bool empty = m_text.empty();
    const std::string& display = empty ? m_placeholder : m_text;

    SkRect bounds;
    font.measureText(display.c_str(), display.size(), SkTextEncoding::kUTF8, &bounds);

    float ty = (float)y + ((float)h - bounds.height()) / 2.f - bounds.top();

    SkPaint textPaint;
    textPaint.setAntiAlias(true);
    textPaint.setColor(empty ? SkColorSetRGB(170, 170, 178) : SkColorSetRGB(20, 20, 30));
    canvas->drawString(display.c_str(), textL - bounds.left(), ty, font, textPaint);

    // Cursor
    if (m_focused && !empty) {
        SkScalar advance = font.measureText(
            m_text.c_str(), m_cursor, SkTextEncoding::kUTF8, nullptr);
        float cx = textL - bounds.left() + advance;

        SkPaint cursor;
        cursor.setAntiAlias(true);
        cursor.setColor(SkColorSetRGB(55, 120, 220));
        cursor.setStrokeWidth(1.5f);
        canvas->drawLine(cx, (float)y + 6.f, cx, (float)y + (float)h - 6.f, cursor);
    } else if (m_focused && empty) {
        SkPaint cursor;
        cursor.setAntiAlias(true);
        cursor.setColor(SkColorSetRGB(55, 120, 220));
        cursor.setStrokeWidth(1.5f);
        canvas->drawLine(textL, (float)y + 6.f, textL, (float)y + (float)h - 6.f, cursor);
    }

    canvas->restore(); // clip
    canvas->restore();
}

void skTextInput::OnEvent(const skEvent& event) {
    switch (event.type) {
        case skEventType::KeyChar: {
            wchar_t ch = event.ch;
            if (ch == 8) {          // Backspace
                if (m_cursor > 0) { m_text.erase(m_cursor - 1, 1); --m_cursor; }
            } else if (ch >= 32) {  // Printable (ASCII + extended)
                m_text.insert(m_cursor, 1, static_cast<char>(ch));
                ++m_cursor;
            }
            if (m_onChange) m_onChange(m_text);
            break;
        }
        case skEventType::KeyDown:
            switch (event.button) {
                case VK_LEFT:   if (m_cursor > 0)            --m_cursor; break;
                case VK_RIGHT:  if (m_cursor < m_text.size()) ++m_cursor; break;
                case VK_HOME:   m_cursor = 0;                break;
                case VK_END:    m_cursor = m_text.size();    break;
                case VK_DELETE:
                    if (m_cursor < m_text.size()) {
                        m_text.erase(m_cursor, 1);
                        if (m_onChange) m_onChange(m_text);
                    }
                    break;
            }
            break;
        default:
            break;
    }
}
