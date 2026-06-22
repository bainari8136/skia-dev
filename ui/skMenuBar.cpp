#include "skMenuBar.h"
#include "skTheme.h"
#include "skTypeface.h"
#include <include/core/SkPaint.h>
#include <include/core/SkRRect.h>
#include <include/core/SkFont.h>

skMenuBar::skMenuBar(int cx, int cy, int cw, int ch)
    : skWidget(cx, cy, cw, ch) {}

void skMenuBar::addMenu(const std::string& title, std::shared_ptr<skMenu> menu) {
    m_entries.push_back({title, std::move(menu), 0.f, 0.f});
    reLayout();
}

void skMenuBar::reLayout() {
    static sk_sp<SkTypeface> tf = skGetSystemTypeface();
    SkFont font(tf, 13.f);
    constexpr float kPadX = 13.f;
    float curX = (float)x + 4.f;
    for (auto& e : m_entries) {
        SkRect tb;
        font.measureText(e.title.c_str(), e.title.size(), SkTextEncoding::kUTF8, &tb);
        e.titleW = tb.width() + kPadX * 2.f;
        e.titleX = curX;
        curX += e.titleW;
    }
}

int skMenuBar::hitTest(int px) const {
    for (int i = 0; i < (int)m_entries.size(); ++i) {
        if ((float)px >= m_entries[i].titleX &&
            (float)px <  m_entries[i].titleX + m_entries[i].titleW)
            return i;
    }
    return -1;
}

void skMenuBar::syncOpen() {
    if (m_openIdx >= 0 && !m_entries[m_openIdx].menu->visible())
        m_openIdx = -1;
}

void skMenuBar::Paint(SkCanvas* canvas) {
    syncOpen();
    const auto& th = skGetTheme();

    // Bottom separator
    SkPaint lp; lp.setColor(th.panelBorder);
    canvas->drawLine((float)x, (float)(y + h),
                     (float)(x + w), (float)(y + h), lp);

    static sk_sp<SkTypeface> tf = skGetSystemTypeface();
    SkFont font(tf, 13.f);
    font.setEdging(SkFont::Edging::kAntiAlias);

    for (int i = 0; i < (int)m_entries.size(); ++i) {
        auto& e = m_entries[i];
        bool active = (i == m_openIdx);
        bool hover  = (i == m_hoverIdx && !active);

        if (active || hover) {
            SkPaint hp; hp.setAntiAlias(true);
            hp.setColor(active
                ? SkColorSetARGB(36, SkColorGetR(th.accent),
                                     SkColorGetG(th.accent),
                                     SkColorGetB(th.accent))
                : SkColorSetARGB(14, 120, 120, 120));
            SkRRect hrr;
            hrr.setRectXY(
                SkRect::MakeXYWH(e.titleX, (float)y + 2.f, e.titleW, (float)h - 4.f),
                4.f, 4.f);
            canvas->drawRRect(hrr, hp);
        }

        SkRect tb;
        font.measureText(e.title.c_str(), e.title.size(), SkTextEncoding::kUTF8, &tb);
        SkPaint tp; tp.setAntiAlias(true);
        tp.setColor(active ? th.accent : th.textPrimary);
        canvas->drawString(e.title.c_str(),
            e.titleX + (e.titleW - tb.width()) * 0.5f - tb.left(),
            (float)y + ((float)h - tb.height()) * 0.5f - tb.top(),
            font, tp);
    }
}

void skMenuBar::OnEvent(const skEvent& ev) {
    syncOpen();

    if (ev.type == skEventType::MouseMove) {
        bool over = (ev.x >= x && ev.x < x + w && ev.y >= y && ev.y < y + h);
        int hit = over ? hitTest(ev.x) : -1;
        m_hoverIdx = hit;
        if (m_openIdx >= 0 && hit >= 0 && hit != m_openIdx) {
            m_entries[m_openIdx].menu->setVisible(false);
            m_openIdx = hit;
            auto& e = m_entries[hit];
            e.menu->openAt((int)e.titleX, y + h + 1);
        }
        return;
    }

    if (ev.type == skEventType::MouseDown) {
        if (ev.x < x || ev.x >= x + w || ev.y < y || ev.y >= y + h) return;
        int hit = hitTest(ev.x);
        if (hit < 0) return;

        if (m_openIdx == hit) {
            m_entries[hit].menu->setVisible(false);
            m_openIdx = -1;
        } else {
            if (m_openIdx >= 0) {
                m_entries[m_openIdx].menu->setVisible(false);
            }
            m_openIdx = hit;
            auto& e = m_entries[hit];
            e.menu->openAt((int)e.titleX, y + h + 1);
        }
    }
}
