#include "skCheckBox.h"
#include "skTypeface.h"
#include <include/core/SkPaint.h>
#include <include/core/SkFont.h>
#include <include/core/SkPath.h>
#include <include/core/SkRRect.h>

skCheckBox::skCheckBox(int cx, int cy, int cw, int ch, std::string label)
    : skWidget(cx, cy, cw, ch), m_label(std::move(label)) {}

void skCheckBox::Paint(SkCanvas* canvas) {
    canvas->save();

    const float boxSize = (float)(h - 4);
    const float boxX    = (float)x + 2.f;
    const float boxY    = (float)y + 2.f;

    // Box background
    SkPaint boxFill;
    boxFill.setAntiAlias(true);
    boxFill.setStyle(SkPaint::kFill_Style);
    if (m_hovered)
        boxFill.setColor(SkColorSetRGB(230, 238, 255));
    else
        boxFill.setColor(SK_ColorWHITE);

    SkRRect box;
    box.setRectXY(SkRect::MakeXYWH(boxX, boxY, boxSize, boxSize), 4.f, 4.f);
    canvas->drawRRect(box, boxFill);

    // Box border
    SkPaint boxBorder;
    boxBorder.setAntiAlias(true);
    boxBorder.setStyle(SkPaint::kStroke_Style);
    boxBorder.setStrokeWidth(1.5f);
    boxBorder.setColor(m_checked ? SkColorSetRGB(55, 120, 220)
                                 : SkColorSetRGB(160, 160, 160));
    canvas->drawRRect(box, boxBorder);

    // Checkmark
    if (m_checked) {
        SkPaint tick;
        tick.setAntiAlias(true);
        tick.setStyle(SkPaint::kStroke_Style);
        tick.setStrokeWidth(2.0f);
        tick.setStrokeCap(SkPaint::kRound_Cap);
        tick.setStrokeJoin(SkPaint::kRound_Join);
        tick.setColor(SkColorSetRGB(55, 120, 220));

        float pad = boxSize * 0.18f;
        float mx  = boxX + boxSize * 0.38f;
        float my  = boxY + boxSize * 0.72f;

        SkPath path;
        path.moveTo(boxX + pad,         boxY + boxSize * 0.50f);
        path.lineTo(mx,                 my);
        path.lineTo(boxX + boxSize - pad, boxY + pad);
        canvas->drawPath(path, tick);
    }

    // Label text
    SkFont font(skGetSystemTypeface(), 16.0f);
    font.setEdging(SkFont::Edging::kAntiAlias);

    SkRect bounds;
    font.measureText(m_label.c_str(), m_label.size(), SkTextEncoding::kUTF8, &bounds);

    float tx = boxX + boxSize + 8.f - bounds.left();
    float ty = (float)y + (h - bounds.height()) / 2.f - bounds.top();

    SkPaint text;
    text.setAntiAlias(true);
    text.setColor(SkColorSetRGB(30, 30, 30));
    canvas->drawString(m_label.c_str(), tx, ty, font, text);

    canvas->restore();
}

void skCheckBox::OnEvent(const skEvent& event) {
    switch (event.type) {
        case skEventType::MouseMove:
            m_hovered = contains(event.x, event.y);
            break;
        case skEventType::MouseDown:
            break;
        case skEventType::MouseUp:
            if (contains(event.x, event.y)) {
                m_checked = !m_checked;
                if (m_onChange) m_onChange(m_checked);
            }
            break;
    }
}
