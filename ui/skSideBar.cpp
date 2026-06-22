#include "skSideBar.h"
#include "skTheme.h"
#include "skTypeface.h"
#include <include/core/SkPaint.h>
#include <include/core/SkFont.h>
#include <include/core/SkRRect.h>

static sk_sp<SkTypeface>& sbTf() {
    static sk_sp<SkTypeface> tf = skGetSystemTypeface();
    return tf;
}

skSideBar::skSideBar(int cx, int cy, int cw, int ch)
    : skWidget(cx, cy, cw, ch) {}

void skSideBar::addItem(const std::string& label) {
    m_items.push_back(label);
}

void skSideBar::setSelected(int idx) {
    if (idx >= 0 && idx < (int)m_items.size()) m_selected = idx;
}

void skSideBar::setOnChange(std::function<void(int, const std::string&)> fn) {
    m_onChange = std::move(fn);
}

int skSideBar::itemAt(int py) const {
    int rel = py - y;
    if (rel < 0) return -1;
    int idx = (int)((float)rel / kItemH);
    return (idx < (int)m_items.size()) ? idx : -1;
}

void skSideBar::Paint(SkCanvas* canvas) {
    const auto& th = skGetTheme();
    SkFont font(sbTf(), 13.f);
    font.setEdging(SkFont::Edging::kAntiAlias);

    // Right border
    SkPaint brd; brd.setColor(th.panelBorder);
    canvas->drawLine((float)(x + w), (float)y, (float)(x + w), (float)(y + h), brd);

    for (int i = 0; i < (int)m_items.size(); ++i) {
        float iy = (float)y + (float)i * kItemH;
        bool active = (i == m_selected);
        bool hover  = (i == m_hoverIdx && !active);

        if (active || hover) {
            SkPaint rp; rp.setAntiAlias(true);
            rp.setColor(active
                ? SkColorSetARGB(22, SkColorGetR(th.accent),
                                     SkColorGetG(th.accent),
                                     SkColorGetB(th.accent))
                : SkColorSetARGB(10, 120, 120, 120));
            canvas->drawRect(SkRect::MakeXYWH((float)x, iy, (float)w, kItemH), rp);
        }

        // Active indicator bar
        if (active) {
            SkRRect irr;
            irr.setRectXY(SkRect::MakeXYWH((float)x, iy + 6.f, kBarW, kItemH - 12.f), 2.f, 2.f);
            SkPaint ip; ip.setAntiAlias(true); ip.setColor(th.accent);
            canvas->drawRRect(irr, ip);
        }

        SkRect tb;
        font.measureText(m_items[i].c_str(), m_items[i].size(), SkTextEncoding::kUTF8, &tb);
        SkPaint tp; tp.setAntiAlias(true);
        tp.setColor(active ? th.accent : th.textPrimary);
        canvas->drawString(m_items[i].c_str(),
            (float)x + kPadX - tb.left(),
            iy + (kItemH - tb.height()) / 2.f - tb.top(),
            font, tp);
    }
}

void skSideBar::OnEvent(const skEvent& ev) {
    if (ev.type == skEventType::MouseMove) {
        m_hoverIdx = (ev.x >= x && ev.x < x + w && ev.y >= y && ev.y < y + h)
            ? itemAt(ev.y) : -1;
        return;
    }
    if (ev.type == skEventType::MouseDown) {
        if (ev.x >= x && ev.x < x + w) {
            int hit = itemAt(ev.y);
            if (hit >= 0) {
                m_selected = hit;
                if (m_onChange) m_onChange(hit, m_items[hit]);
            }
        }
    }
}
