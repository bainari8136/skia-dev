#include "skTabBar.h"
#include "skTypeface.h"
#include "skTheme.h"
#include <include/core/SkPaint.h>
#include <include/core/SkFont.h>
#include <include/core/SkPath.h>

skTabBar::skTabBar(int tx, int ty, int tw, int th)
    : skWidget(tx, ty, tw, th) {}

void skTabBar::addTab(std::string label) { m_tabs.push_back(std::move(label)); }

int skTabBar::hitTab(int mx) const {
    if (m_tabs.empty()) return -1;
    float tw = tabWidth();
    int idx = (int)((mx - x) / tw);
    return (idx >= 0 && idx < (int)m_tabs.size()) ? idx : -1;
}

void skTabBar::Paint(SkCanvas* canvas) {
    if (m_tabs.empty()) return;
    const auto& th = skGetTheme();
    const float tw = tabWidth();

    static sk_sp<SkTypeface> s_tf = skGetSystemTypeface();
    SkFont font(s_tf, 13.f);
    font.setEdging(SkFont::Edging::kAntiAlias);

    for (int i = 0; i < (int)m_tabs.size(); ++i) {
        float tx = (float)x + i * tw;

        // Hover background
        if (i == m_hovered && i != m_selected) {
            SkPaint hov;
            hov.setColor(SkColorSetARGB(15,
                SkColorGetR(th.accent), SkColorGetG(th.accent), SkColorGetB(th.accent)));
            canvas->drawRect(SkRect::MakeXYWH(tx, (float)y, tw, (float)h), hov);
        }

        // Tab label
        SkRect lb;
        font.measureText(m_tabs[i].c_str(), m_tabs[i].size(), SkTextEncoding::kUTF8, &lb);
        float labelX = tx + (tw - lb.width()) / 2.f - lb.left();
        float labelY = (float)y + ((float)h - lb.height()) / 2.f - lb.top();

        SkPaint tp;
        tp.setAntiAlias(true);
        tp.setColor(i == m_selected ? th.accent : th.textSecondary);
        canvas->drawString(m_tabs[i].c_str(), labelX, labelY, font, tp);

        // Active underline
        if (i == m_selected) {
            SkPaint ul;
            ul.setAntiAlias(true);
            ul.setColor(th.accent);
            ul.setStrokeWidth(2.5f);
            ul.setStrokeCap(SkPaint::kRound_Cap);
            float ux = tx + tw * 0.15f;
            float uw = tw * 0.70f;
            float uy = (float)(y + h) - 1.5f;
            canvas->drawLine(ux, uy, ux + uw, uy, ul);
        }
    }

    // Bottom divider
    SkPaint div;
    div.setColor(th.panelBorder);
    canvas->drawLine((float)x, (float)(y + h), (float)(x + w), (float)(y + h), div);
}

void skTabBar::OnEvent(const skEvent& event) {
    switch (event.type) {
        case skEventType::MouseMove:
            m_hovered = contains(event.x, event.y) ? hitTab(event.x) : -1;
            break;
        case skEventType::MouseDown:
            if (contains(event.x, event.y)) {
                int idx = hitTab(event.x);
                if (idx >= 0 && idx != m_selected) {
                    m_selected = idx;
                    if (m_onChange) m_onChange(idx, m_tabs[idx]);
                }
            }
            break;
        default: break;
    }
}
