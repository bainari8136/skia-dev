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

std::shared_ptr<skLabel> skLabel::make(int lx, int ly, int lw, int lh, std::string t, float fontSize) {
    return std::make_shared<skLabel>(lx, ly, lw, lh, std::move(t), fontSize);
}
std::shared_ptr<skLabel> skLabel::text(std::string t)   { m_text = std::move(t); return shared_this(); }
std::shared_ptr<skLabel> skLabel::fontSize(float s)     { m_fontSize = s; return shared_this(); }
std::shared_ptr<skLabel> skLabel::color(SkColor c)      { m_color = c; m_colorSet = true; return shared_this(); }
std::shared_ptr<skLabel> skLabel::pos(int px, int py)   { x = px; y = py; return shared_this(); }
std::shared_ptr<skLabel> skLabel::size(int pw, int ph)  { w = pw; h = ph; return shared_this(); }
