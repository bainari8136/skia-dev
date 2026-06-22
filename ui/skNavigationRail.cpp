#include "skNavigationRail.h"
#include "skTheme.h"
#include "skTypeface.h"
#include <include/core/SkPaint.h>
#include <include/core/SkFont.h>
#include <include/core/SkRRect.h>

static sk_sp<SkTypeface>& nrTf() {
    static sk_sp<SkTypeface> tf = skGetSystemTypeface();
    return tf;
}

skNavigationRail::skNavigationRail(int rx, int ry, int rw, int rh)
    : skWidget(rx, ry, rw, rh) {}

void skNavigationRail::addItem(const std::string& icon, const std::string& label,
                               std::function<void()> action) {
    m_items.push_back({icon, label, std::move(action)});
}

void skNavigationRail::setSelected(int idx) {
    if (idx >= 0 && idx < (int)m_items.size()) m_selected = idx;
}

void skNavigationRail::setOnChange(std::function<void(int, const std::string&)> fn) {
    m_onChange = std::move(fn);
}

int skNavigationRail::itemAt(int py) const {
    int rel = py - y;
    if (rel < 0) return -1;
    int idx = (int)((float)rel / kItemH);
    return (idx < (int)m_items.size()) ? idx : -1;
}

void skNavigationRail::Paint(SkCanvas* canvas) {
    const auto& th = skGetTheme();

    SkPaint bg; bg.setColor(th.panelBg);
    canvas->drawRect(SkRect::MakeXYWH((float)x,(float)y,(float)w,(float)h), bg);

    SkPaint brd; brd.setColor(th.panelBorder);
    canvas->drawLine((float)(x+w),(float)y,(float)(x+w),(float)(y+h), brd);

    SkFont iconFont(nrTf(), kIconSz); iconFont.setEdging(SkFont::Edging::kAntiAlias);
    SkFont lblFont(nrTf(), 10.f);    lblFont.setEdging(SkFont::Edging::kAntiAlias);

    for (int i = 0; i < (int)m_items.size(); ++i) {
        float iy = (float)y + (float)i * kItemH;
        bool active = (i == m_selected);
        bool hover  = (i == m_hoverIdx && !active);

        if (active) {
            SkRRect pill;
            pill.setRectXY(SkRect::MakeXYWH((float)x + 8.f, iy + 10.f, (float)w - 16.f, 32.f), 16.f, 16.f);
            SkPaint pp; pp.setAntiAlias(true);
            pp.setColor(SkColorSetARGB(28, SkColorGetR(th.accent), SkColorGetG(th.accent), SkColorGetB(th.accent)));
            canvas->drawRRect(pill, pp);
        } else if (hover) {
            SkPaint hp; hp.setColor(SkColorSetARGB(10, 120, 120, 120));
            canvas->drawRect(SkRect::MakeXYWH((float)x, iy, (float)w, kItemH), hp);
        }

        if (active) {
            SkRRect bar;
            bar.setRectXY(SkRect::MakeXYWH((float)x, iy + kItemH * 0.33f, kBarW, kItemH * 0.34f), 2.f, 2.f);
            SkPaint bp; bp.setAntiAlias(true); bp.setColor(th.accent);
            canvas->drawRRect(bar, bp);
        }

        SkColor itemColor = active ? th.accent : th.textSecondary;

        // Icon
        {
            SkRect ib;
            iconFont.measureText(m_items[i].icon.c_str(), m_items[i].icon.size(), SkTextEncoding::kUTF8, &ib);
            SkPaint ip; ip.setAntiAlias(true); ip.setColor(itemColor);
            canvas->drawString(m_items[i].icon.c_str(),
                (float)x + ((float)w - ib.width()) / 2.f - ib.left(),
                iy + 16.f - ib.top(), iconFont, ip);
        }

        // Label
        {
            SkRect lb;
            lblFont.measureText(m_items[i].label.c_str(), m_items[i].label.size(), SkTextEncoding::kUTF8, &lb);
            SkPaint lp; lp.setAntiAlias(true); lp.setColor(itemColor);
            canvas->drawString(m_items[i].label.c_str(),
                (float)x + ((float)w - lb.width()) / 2.f - lb.left(),
                iy + 40.f - lb.top(), lblFont, lp);
        }
    }
}

void skNavigationRail::OnEvent(const skEvent& ev) {
    if (ev.type == skEventType::MouseMove) {
        m_hoverIdx = (ev.x >= x && ev.x < x + w && ev.y >= y && ev.y < y + h)
            ? itemAt(ev.y) : -1;
        return;
    }
    if (ev.type == skEventType::MouseDown && ev.x >= x && ev.x < x + w) {
        int hit = itemAt(ev.y);
        if (hit >= 0) {
            m_selected = hit;
            if (m_items[hit].action) m_items[hit].action();
            if (m_onChange) m_onChange(hit, m_items[hit].label);
        }
    }
}
