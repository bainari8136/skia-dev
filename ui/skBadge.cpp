#include "skBadge.h"
#include "skTypeface.h"
#include "skTheme.h"
#include <include/core/SkPaint.h>
#include <include/core/SkFont.h>
#include <include/core/SkRRect.h>

skBadge::skBadge(int bx, int by, std::string text, float fontSize)
    : skWidget(bx, by, 0, 0), m_text(std::move(text)), m_fontSize(fontSize) {
    // Auto-size at construction time
    sk_sp<SkTypeface> tf = skGetSystemTypeface();
    SkFont font(tf, m_fontSize);
    SkRect bounds;
    font.measureText(m_text.c_str(), m_text.size(), SkTextEncoding::kUTF8, &bounds);
    const float padX = 9.f, padY = 3.f;
    w = (int)(bounds.width()  + padX * 2.f + 0.5f);
    h = (int)(bounds.height() + padY * 2.f + 0.5f);
}

void skBadge::Paint(SkCanvas* canvas) {
    const auto& th = skGetTheme();
    SkColor bg = m_colorSet ? m_bg : th.accent;
    SkColor fg = m_fg;

    // Pill shape (fully rounded)
    SkPaint fill;
    fill.setAntiAlias(true);
    fill.setColor(bg);
    SkRRect rr;
    float radius = (float)h / 2.f;
    rr.setRectXY(SkRect::MakeXYWH((float)x, (float)y, (float)w, (float)h), radius, radius);
    canvas->drawRRect(rr, fill);

    // Label
    sk_sp<SkTypeface> tf = skGetSystemTypeface();
    SkFont font(tf, m_fontSize);
    font.setEdging(SkFont::Edging::kAntiAlias);

    SkRect bounds;
    font.measureText(m_text.c_str(), m_text.size(), SkTextEncoding::kUTF8, &bounds);
    float tx = (float)x + ((float)w - bounds.width()) / 2.f - bounds.left();
    float ty = (float)y + ((float)h - bounds.height()) / 2.f - bounds.top();

    SkPaint tp;
    tp.setAntiAlias(true);
    tp.setColor(fg);
    canvas->drawString(m_text.c_str(), tx, ty, font, tp);
}
