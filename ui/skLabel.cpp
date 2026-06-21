#include "skLabel.h"
#include "skTypeface.h"
#include "skTheme.h"
#include <include/core/SkPaint.h>
#include <include/core/SkFont.h>

skLabel::skLabel(int lx, int ly, int lw, int lh, std::string text, float fontSize)
    : skWidget(lx, ly, lw, lh), m_text(std::move(text)), m_fontSize(fontSize) {}

void skLabel::Paint(SkCanvas* canvas) {
    if (m_text.empty()) return;
    canvas->save();

    SkFont font(skGetSystemTypeface(), m_fontSize);
    font.setEdging(SkFont::Edging::kAntiAlias);

    SkRect bounds;
    font.measureText(m_text.c_str(), m_text.size(), SkTextEncoding::kUTF8, &bounds);

    SkPaint paint;
    paint.setAntiAlias(true);
    paint.setColor(m_colorSet ? m_color : skGetTheme().textPrimary);
    canvas->drawString(m_text.c_str(), (float)x - bounds.left(), (float)y - bounds.top(), font, paint);

    canvas->restore();
}
