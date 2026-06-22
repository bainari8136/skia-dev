#include <Windows.h>
#include "skDrawer.h"
#include "skTheme.h"
#include "skTypeface.h"
#include <include/core/SkPaint.h>
#include <include/core/SkFont.h>
#include <cmath>

static sk_sp<SkTypeface>& drTf() {
    static sk_sp<SkTypeface> tf = skGetSystemTypeface();
    return tf;
}

skDrawer::skDrawer(int totalW, int totalH, int drawerW)
    : skWidget(0, 0, totalW, totalH), m_drawerW(drawerW) {
    setVisible(false);
}

void skDrawer::addItem(const std::string& label, std::function<void()> action) {
    m_items.push_back({label, std::move(action)});
}

void skDrawer::open() {
    m_open = true;
    setVisible(true);
}

void skDrawer::close() {
    m_open = false;
}

int skDrawer::itemAt(int px, int py) const {
    float pl = panelLeft();
    if (px < (int)pl || px >= (int)pl + m_drawerW) return -1;
    float rel = (float)py - kTitleH;
    if (rel < 0) return -1;
    int idx = (int)(rel / kItemH);
    return (idx < (int)m_items.size()) ? idx : -1;
}

void skDrawer::onTick() {
    float target = m_open ? 1.f : 0.f;
    float diff   = target - m_offset;
    if (std::fabsf(diff) < 0.015f) {
        m_offset = target;
        if (!m_open) setVisible(false);
    } else {
        m_offset += diff * 0.55f;
    }
}

void skDrawer::Paint(SkCanvas* canvas) {
    const auto& th = skGetTheme();
    float pl = panelLeft();

    // Semi-transparent backdrop
    uint8_t alpha = (uint8_t)(m_offset * 110.f);
    SkPaint bkp; bkp.setColor(SkColorSetARGB(alpha, 0, 0, 0));
    canvas->drawRect(SkRect::MakeXYWH(0.f, 0.f, (float)w, (float)h), bkp);

    // Soft shadow on panel right edge
    SkPaint shp; shp.setColor(SkColorSetARGB(36, 0, 0, 0));
    canvas->drawRect(SkRect::MakeXYWH(pl + (float)m_drawerW, 0.f, 8.f, (float)h), shp);

    // Panel background
    SkPaint panelBg; panelBg.setColor(th.panelBg);
    canvas->drawRect(SkRect::MakeXYWH(pl, 0.f, (float)m_drawerW, (float)h), panelBg);

    // Accent title bar
    SkPaint titleBg; titleBg.setColor(th.accent);
    canvas->drawRect(SkRect::MakeXYWH(pl, 0.f, (float)m_drawerW, kTitleH), titleBg);

    if (!m_title.empty()) {
        SkFont tf(drTf(), 15.f); tf.setEdging(SkFont::Edging::kAntiAlias);
        SkRect tb;
        tf.measureText(m_title.c_str(), m_title.size(), SkTextEncoding::kUTF8, &tb);
        SkPaint tp; tp.setAntiAlias(true); tp.setColor(SK_ColorWHITE);
        canvas->drawString(m_title.c_str(),
            pl + 16.f - tb.left(), kTitleH / 2.f - tb.height() / 2.f - tb.top(), tf, tp);
    }

    SkFont itemFont(drTf(), 13.f); itemFont.setEdging(SkFont::Edging::kAntiAlias);
    for (int i = 0; i < (int)m_items.size(); ++i) {
        float iy = kTitleH + (float)i * kItemH;

        if (i == m_hoverIdx) {
            SkPaint hp; hp.setColor(SkColorSetARGB(12, 120, 120, 120));
            canvas->drawRect(SkRect::MakeXYWH(pl, iy, (float)m_drawerW, kItemH), hp);
        }

        SkPaint div; div.setColor(th.panelBorder);
        canvas->drawLine(pl + 12.f, iy, pl + (float)m_drawerW - 12.f, iy, div);

        SkRect ib;
        itemFont.measureText(m_items[i].label.c_str(), m_items[i].label.size(), SkTextEncoding::kUTF8, &ib);
        SkPaint ip; ip.setAntiAlias(true); ip.setColor(th.textPrimary);
        canvas->drawString(m_items[i].label.c_str(),
            pl + 16.f - ib.left(), iy + (kItemH - ib.height()) / 2.f - ib.top(),
            itemFont, ip);
    }

    // Right border
    SkPaint rbrd; rbrd.setColor(th.panelBorder);
    canvas->drawLine(pl + (float)m_drawerW, 0.f, pl + (float)m_drawerW, (float)h, rbrd);
}

bool skDrawer::handleEvent(const skEvent& ev) {
    if (!visible()) return false;

    if (ev.type == skEventType::KeyDown && ev.button == VK_ESCAPE) {
        close(); return true;
    }

    float pl = panelLeft();
    bool inPanel = (ev.x >= (int)pl && ev.x < (int)pl + m_drawerW);

    if (ev.type == skEventType::MouseMove) {
        m_hoverIdx = itemAt(ev.x, ev.y);
        return inPanel;
    }
    if (ev.type == skEventType::MouseDown) {
        if (!inPanel) { close(); return true; }
        int hit = itemAt(ev.x, ev.y);
        if (hit >= 0 && m_items[hit].action) { m_items[hit].action(); close(); }
        return true;
    }
    return inPanel;
}
