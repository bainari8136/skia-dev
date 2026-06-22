#include "skBreadcrumb.h"
#include "skTheme.h"
#include "skTypeface.h"
#include <include/core/SkPaint.h>
#include <include/core/SkFont.h>

static sk_sp<SkTypeface>& bcTf() {
    static sk_sp<SkTypeface> tf = skGetSystemTypeface();
    return tf;
}

skBreadcrumb::skBreadcrumb(int cx, int cy, int cw, int ch)
    : skWidget(cx, cy, cw, ch) {}

void skBreadcrumb::addCrumb(const std::string& label) {
    m_crumbs.push_back({label, 0.f, 0.f});
    reLayout();
}

void skBreadcrumb::clear() {
    m_crumbs.clear();
    m_hoverIdx = -1;
}

void skBreadcrumb::setOnClick(std::function<void(int, const std::string&)> fn) {
    m_onClick = std::move(fn);
}

void skBreadcrumb::reLayout() {
    SkFont font(bcTf(), kFontSize);
    float curX = (float)x + 4.f;
    for (auto& c : m_crumbs) {
        c.crumbX = curX;
        SkRect tb;
        font.measureText(c.label.c_str(), c.label.size(), SkTextEncoding::kUTF8, &tb);
        c.crumbW = tb.width() + 4.f;
        curX += c.crumbW + kSepW;
    }
}

int skBreadcrumb::hitTest(int px) const {
    for (int i = 0; i + 1 < (int)m_crumbs.size(); ++i) {
        const auto& c = m_crumbs[i];
        if ((float)px >= c.crumbX && (float)px < c.crumbX + c.crumbW)
            return i;
    }
    return -1;
}

void skBreadcrumb::Paint(SkCanvas* canvas) {
    if (m_crumbs.empty()) return;
    const auto& th = skGetTheme();
    SkFont font(bcTf(), kFontSize);
    font.setEdging(SkFont::Edging::kAntiAlias);

    for (int i = 0; i < (int)m_crumbs.size(); ++i) {
        bool isLast = (i == (int)m_crumbs.size() - 1);
        const auto& c = m_crumbs[i];

        SkRect tb;
        font.measureText(c.label.c_str(), c.label.size(), SkTextEncoding::kUTF8, &tb);
        float ty = (float)y + ((float)h - tb.height()) / 2.f - tb.top();

        SkPaint tp; tp.setAntiAlias(true);
        if (isLast)
            tp.setColor(th.textPrimary);
        else if (i == m_hoverIdx)
            tp.setColor(th.accentHover);
        else
            tp.setColor(th.accent);

        canvas->drawString(c.label.c_str(), c.crumbX - tb.left(), ty, font, tp);

        // Underline on clickable (non-last) crumbs
        if (!isLast) {
            SkPaint ul; ul.setColor(i == m_hoverIdx ? th.accentHover : th.accent);
            ul.setStrokeWidth(0.75f);
            canvas->drawLine(c.crumbX, ty + tb.height() + 1.f,
                             c.crumbX + c.crumbW, ty + tb.height() + 1.f, ul);

            // Separator "›"
            const char* sep = "\xe2\x80\xba"; // UTF-8 ›
            SkRect sb;
            font.measureText(sep, strlen(sep), SkTextEncoding::kUTF8, &sb);
            SkPaint sp; sp.setAntiAlias(true); sp.setColor(th.textSecondary);
            canvas->drawString(sep,
                c.crumbX + c.crumbW + (kSepW - sb.width()) / 2.f - sb.left(),
                ty, font, sp);
        }
    }
}

void skBreadcrumb::OnEvent(const skEvent& ev) {
    if (ev.type == skEventType::MouseMove) {
        m_hoverIdx = (ev.x >= x && ev.x < x + w && ev.y >= y && ev.y < y + h)
            ? hitTest(ev.x) : -1;
        return;
    }
    if (ev.type == skEventType::MouseDown) {
        int hit = hitTest(ev.x);
        if (hit >= 0 && m_onClick) m_onClick(hit, m_crumbs[hit].label);
    }
}
