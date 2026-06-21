#include "skStatusBar.h"
#include "skTypeface.h"
#include "skTheme.h"
#include <include/core/SkPaint.h>
#include <include/core/SkFont.h>

skStatusBar::skStatusBar(int sx, int sy, int sw, int sh)
    : skWidget(sx, sy, sw, sh), m_left("Ready") {}

void skStatusBar::setText(const std::string& left, const std::string& right) {
    m_left  = left;
    m_right = right;
}

void skStatusBar::Paint(SkCanvas* canvas) {
    const auto& th = skGetTheme();

    // Background
    SkPaint bg;
    bg.setColor(th.panelBg);
    canvas->drawRect(SkRect::MakeXYWH((float)x, (float)y, (float)w, (float)h), bg);

    // Top border
    SkPaint brd;
    brd.setColor(th.panelBorder);
    canvas->drawLine((float)x, (float)y, (float)(x + w), (float)y, brd);

    // Accent left stripe
    SkPaint stripe;
    stripe.setColor(th.accent);
    canvas->drawRect(SkRect::MakeXYWH((float)x, (float)y, 3.f, (float)h), stripe);

    static sk_sp<SkTypeface> s_tf = skGetSystemTypeface();
    SkFont font(s_tf, 11.f);
    font.setEdging(SkFont::Edging::kAntiAlias);

    SkPaint tp;
    tp.setAntiAlias(true);
    tp.setColor(th.textSecondary);

    float textY = (float)y + (float)h / 2.f + 4.f;

    // Left text
    if (!m_left.empty()) {
        canvas->drawString(m_left.c_str(), (float)x + 12.f, textY, font, tp);
    }

    // Right text
    if (!m_right.empty()) {
        SkRect rb;
        font.measureText(m_right.c_str(), m_right.size(), SkTextEncoding::kUTF8, &rb);
        canvas->drawString(m_right.c_str(),
            (float)(x + w) - rb.width() - rb.left() - 12.f,
            textY, font, tp);
    }
}
