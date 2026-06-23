#include "skLink.h"
#include "skTypeface.h"
#include "skTheme.h"
#include <include/core/SkPaint.h>
#include <include/core/SkFont.h>

skLink::skLink(int lx, int ly, std::string text, float fontSize)
    : skWidget(lx, ly, 0, 0), m_text(std::move(text)), m_fontSize(fontSize) {
    // Auto-size to text bounds
    sk_sp<SkTypeface> tf = skGetSystemTypeface();
    SkFont font(tf, m_fontSize);
    SkRect b;
    font.measureText(m_text.c_str(), m_text.size(), SkTextEncoding::kUTF8, &b);
    w = (int)(b.width()  + 1.f);
    h = (int)(b.height() + 4.f); // 4px extra for the underline
}

void skLink::Paint(SkCanvas* canvas) {
    const auto& th = skGetTheme();
    SkColor color = m_hovered ? th.accentHover : th.accent;

    sk_sp<SkTypeface> tf = skGetSystemTypeface();
    SkFont font(tf, m_fontSize);
    font.setEdging(SkFont::Edging::kAntiAlias);

    SkRect bounds;
    font.measureText(m_text.c_str(), m_text.size(), SkTextEncoding::kUTF8, &bounds);

    float tx = (float)x - bounds.left();
    float ty = (float)y - bounds.top();

    SkPaint tp;
    tp.setAntiAlias(true);
    tp.setColor(color);
    canvas->drawString(m_text.c_str(), tx, ty, font, tp);

    // Underline (always shown, not just on hover)
    SkPaint ul;
    ul.setAntiAlias(false);
    ul.setColor(color);
    float ulY = (float)y + bounds.height() + 1.f;
    canvas->drawLine((float)x, ulY, (float)x + bounds.width(), ulY, ul);
}

void skLink::OnEvent(const skEvent& event) {
    switch (event.type) {
        case skEventType::MouseMove:
            m_hovered = contains(event.x, event.y);
            break;
        case skEventType::MouseUp:
            if (m_hovered && m_onClick) m_onClick();
            break;
        default: break;
    }
}

std::shared_ptr<skLink> skLink::make(int lx, int ly, std::string text, float fontSize) {
    return std::make_shared<skLink>(lx, ly, std::move(text), fontSize);
}
std::shared_ptr<skLink> skLink::onClick(std::function<void()> cb) { m_onClick = std::move(cb); return shared_this(); }
std::shared_ptr<skLink> skLink::pos(int px, int py)               { x = px; y = py; return shared_this(); }
