#include "skChip.h"
#include "skTheme.h"
#include "skTypeface.h"
#include <include/core/SkPaint.h>
#include <include/core/SkFont.h>
#include <include/core/SkRRect.h>

skChip::skChip(int cx, int cy, const std::string& label, float fontSize)
    : skWidget(cx, cy, 0, 0), m_label(label), m_fontSize(fontSize) {
    sk_sp<SkTypeface> tf = skGetSystemTypeface();
    SkFont font(tf, m_fontSize);
    SkRect bounds;
    font.measureText(m_label.c_str(), m_label.size(), SkTextEncoding::kUTF8, &bounds);
    const float padX = 12.f, padY = 5.f;
    w = (int)(bounds.width()  + padX * 2.f + 0.5f);
    h = (int)(bounds.height() + padY * 2.f + 0.5f);
}

void skChip::Paint(SkCanvas* canvas) {
    const auto& th = skGetTheme();
    SkColor bg = (m_bg != SK_ColorTRANSPARENT) ? m_bg
        : SkColorSetARGB(m_hover && m_onClick ? 45 : 22,
              SkColorGetR(th.accent), SkColorGetG(th.accent), SkColorGetB(th.accent));
    SkColor fg = (m_fg != SK_ColorTRANSPARENT) ? m_fg : th.accent;

    float radius = (float)h / 2.f;
    SkRRect rr;
    rr.setRectXY(SkRect::MakeXYWH((float)x, (float)y, (float)w, (float)h), radius, radius);

    SkPaint fill; fill.setAntiAlias(true); fill.setColor(bg);
    canvas->drawRRect(rr, fill);

    SkPaint brd; brd.setAntiAlias(true);
    brd.setStyle(SkPaint::kStroke_Style);
    brd.setStrokeWidth(1.f);
    brd.setColor(SkColorSetARGB(60,
        SkColorGetR(th.accent), SkColorGetG(th.accent), SkColorGetB(th.accent)));
    canvas->drawRRect(rr, brd);

    sk_sp<SkTypeface> tf = skGetSystemTypeface();
    SkFont font(tf, m_fontSize);
    font.setEdging(SkFont::Edging::kAntiAlias);

    SkRect bounds;
    font.measureText(m_label.c_str(), m_label.size(), SkTextEncoding::kUTF8, &bounds);
    SkPaint tp; tp.setAntiAlias(true); tp.setColor(fg);
    canvas->drawString(m_label.c_str(),
        (float)x + ((float)w - bounds.width()) / 2.f - bounds.left(),
        (float)y + ((float)h - bounds.height()) / 2.f - bounds.top(),
        font, tp);
}

void skChip::OnEvent(const skEvent& ev) {
    if (ev.type == skEventType::MouseMove)
        m_hover = (ev.x >= x && ev.x < x + w && ev.y >= y && ev.y < y + h);
    if (ev.type == skEventType::MouseDown)
        if (ev.x >= x && ev.x < x + w && ev.y >= y && ev.y < y + h)
            if (m_onClick) m_onClick();
}
