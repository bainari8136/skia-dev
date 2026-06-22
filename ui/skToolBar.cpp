#include "skToolBar.h"
#include "skTheme.h"
#include "skTypeface.h"
#include <include/core/SkPaint.h>
#include <include/core/SkFont.h>
#include <include/core/SkRRect.h>

static sk_sp<SkTypeface>& tbTf() {
    static sk_sp<SkTypeface> tf = skGetSystemTypeface();
    return tf;
}

skToolBar::skToolBar(int cx, int cy, int cw, int ch)
    : skWidget(cx, cy, cw, ch) {}

void skToolBar::addItem(const std::string& label, std::function<void()> action) {
    m_items.push_back({label, std::move(action), false, 0.f, 0.f});
    reLayout();
}

void skToolBar::addSeparator() {
    m_items.push_back({"", nullptr, true, 0.f, 1.f + kSepMX * 2.f});
    reLayout();
}

void skToolBar::reLayout() {
    SkFont font(tbTf(), 13.f);
    float curX = (float)x + 4.f;
    for (auto& it : m_items) {
        it.btnX = curX;
        if (it.isSep) {
            it.btnW = 1.f + kSepMX * 2.f;
        } else {
            SkRect tb;
            font.measureText(it.label.c_str(), it.label.size(), SkTextEncoding::kUTF8, &tb);
            it.btnW = tb.width() + kPadX * 2.f;
        }
        curX += it.btnW;
    }
}

int skToolBar::hitTest(int px) const {
    for (int i = 0; i < (int)m_items.size(); ++i) {
        const auto& it = m_items[i];
        if (!it.isSep && (float)px >= it.btnX && (float)px < it.btnX + it.btnW)
            return i;
    }
    return -1;
}

void skToolBar::Paint(SkCanvas* canvas) {
    const auto& th = skGetTheme();

    // Bottom border
    SkPaint lp; lp.setColor(th.panelBorder);
    canvas->drawLine((float)x, (float)(y + h), (float)(x + w), (float)(y + h), lp);

    SkFont font(tbTf(), 13.f);
    font.setEdging(SkFont::Edging::kAntiAlias);

    for (int i = 0; i < (int)m_items.size(); ++i) {
        const auto& it = m_items[i];
        if (it.isSep) {
            SkPaint sp; sp.setColor(th.panelBorder);
            float sx = it.btnX + kSepMX;
            canvas->drawLine(sx, (float)y + 4.f, sx, (float)(y + h) - 4.f, sp);
            continue;
        }

        if (i == m_hoverIdx) {
            SkRRect hrr;
            hrr.setRectXY(
                SkRect::MakeXYWH(it.btnX + 1.f, (float)y + 2.f, it.btnW - 2.f, (float)h - 4.f),
                4.f, 4.f);
            SkPaint hp; hp.setAntiAlias(true);
            hp.setColor(SkColorSetARGB(14, 120, 120, 120));
            canvas->drawRRect(hrr, hp);
        }

        SkRect tb;
        font.measureText(it.label.c_str(), it.label.size(), SkTextEncoding::kUTF8, &tb);
        SkPaint tp; tp.setAntiAlias(true); tp.setColor(th.textPrimary);
        canvas->drawString(it.label.c_str(),
            it.btnX + (it.btnW - tb.width()) / 2.f - tb.left(),
            (float)y + ((float)h - tb.height()) / 2.f - tb.top(),
            font, tp);
    }
}

void skToolBar::OnEvent(const skEvent& ev) {
    if (ev.type == skEventType::MouseMove) {
        m_hoverIdx = (ev.x >= x && ev.x < x + w && ev.y >= y && ev.y < y + h)
            ? hitTest(ev.x) : -1;
        return;
    }
    if (ev.type == skEventType::MouseDown) {
        if (ev.x >= x && ev.x < x + w && ev.y >= y && ev.y < y + h) {
            int hit = hitTest(ev.x);
            if (hit >= 0 && m_items[hit].action) m_items[hit].action();
        }
    }
}
