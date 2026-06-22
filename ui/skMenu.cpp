#include "skMenu.h"
#include "skTheme.h"
#include "skTypeface.h"
#include <include/core/SkPaint.h>
#include <include/core/SkRRect.h>
#include <include/core/SkFont.h>
#include <Windows.h>

static sk_sp<SkTypeface>& menuTf() {
    static sk_sp<SkTypeface> tf = skGetSystemTypeface();
    return tf;
}

skMenu::skMenu() : skWidget(0, 0, 0, 0) {
    setVisible(false);
}

void skMenu::addItem(const std::string& label, std::function<void()> action, bool enabled) {
    m_items.push_back({label, std::move(action), enabled, false});
}

void skMenu::addSeparator() {
    m_items.push_back({"", nullptr, false, true});
}

void skMenu::computeSize() {
    SkFont font(menuTf(), 13.f);
    float maxW = kMinW;
    for (auto& it : m_items) {
        if (it.isSeparator) continue;
        SkRect tb;
        font.measureText(it.label.c_str(), it.label.size(), SkTextEncoding::kUTF8, &tb);
        maxW = std::max(maxW, tb.width() + kPadX * 2.f);
    }
    float totalH = 4.f;
    for (auto& it : m_items)
        totalH += it.isSeparator ? kSepH : kItemH;
    totalH += 4.f;
    w = (int)maxW;
    h = (int)totalH;
}

void skMenu::openAt(int ox, int oy) {
    computeSize();
    x = ox;
    y = oy;
    m_hoverFlat = -1;
    setVisible(true);
}

int skMenu::flatAt(int py) const {
    float curY = (float)y + 4.f;
    for (int i = 0; i < (int)m_items.size(); ++i) {
        float ih = m_items[i].isSeparator ? kSepH : kItemH;
        if ((float)py >= curY && (float)py < curY + ih) return i;
        curY += ih;
    }
    return -1;
}

void skMenu::Paint(SkCanvas* canvas) {
    const auto& th = skGetTheme();

    // Drop shadow
    SkRRect shadowRR;
    shadowRR.setRectXY(
        SkRect::MakeXYWH((float)x + 2, (float)y + 4, (float)w, (float)h), 6.f, 6.f);
    SkPaint sh; sh.setAntiAlias(true); sh.setColor(SkColorSetARGB(22, 0, 0, 0));
    canvas->drawRRect(shadowRR, sh);

    SkRRect shadowRR2;
    shadowRR2.setRectXY(
        SkRect::MakeXYWH((float)x + 1, (float)y + 2, (float)w, (float)h), 6.f, 6.f);
    sh.setColor(SkColorSetARGB(12, 0, 0, 0));
    canvas->drawRRect(shadowRR2, sh);

    // Background
    SkRRect rr;
    rr.setRectXY(SkRect::MakeXYWH((float)x, (float)y, (float)w, (float)h), 6.f, 6.f);
    SkPaint bg; bg.setAntiAlias(true); bg.setColor(th.panelBg);
    canvas->drawRRect(rr, bg);

    SkPaint brd; brd.setAntiAlias(true);
    brd.setStyle(SkPaint::kStroke_Style);
    brd.setStrokeWidth(0.75f);
    brd.setColor(th.panelBorder);
    canvas->drawRRect(rr, brd);

    SkFont font(menuTf(), 13.f);
    font.setEdging(SkFont::Edging::kAntiAlias);

    float curY = (float)y + 4.f;
    for (int i = 0; i < (int)m_items.size(); ++i) {
        auto& it = m_items[i];

        if (it.isSeparator) {
            SkPaint sp; sp.setColor(th.panelBorder);
            canvas->drawLine((float)x + 10.f, curY + kSepH * 0.5f,
                             (float)(x + w) - 10.f, curY + kSepH * 0.5f, sp);
            curY += kSepH;
            continue;
        }

        if (i == m_hoverFlat && it.enabled) {
            SkPaint hp; hp.setAntiAlias(true);
            hp.setColor(SkColorSetARGB(32,
                SkColorGetR(th.accent), SkColorGetG(th.accent), SkColorGetB(th.accent)));
            SkRRect hrr;
            hrr.setRectXY(
                SkRect::MakeXYWH((float)x + 4.f, curY + 1.f, (float)w - 8.f, kItemH - 2.f),
                4.f, 4.f);
            canvas->drawRRect(hrr, hp);
        }

        SkRect tb;
        font.measureText(it.label.c_str(), it.label.size(), SkTextEncoding::kUTF8, &tb);
        SkPaint tp; tp.setAntiAlias(true);
        tp.setColor(it.enabled ? th.textPrimary : th.textSecondary);
        canvas->drawString(it.label.c_str(),
            (float)x + kPadX - tb.left(),
            curY + (kItemH - tb.height()) * 0.5f - tb.top(),
            font, tp);

        curY += kItemH;
    }
}

bool skMenu::handleEvent(const skEvent& ev) {
    if (!visible()) return false;

    if (ev.type == skEventType::KeyDown && ev.button == VK_ESCAPE) {
        setVisible(false);
        return true;
    }

    if (ev.type == skEventType::MouseMove) {
        if (ev.x >= x && ev.x < x + w && ev.y >= y && ev.y < y + h)
            m_hoverFlat = flatAt(ev.y);
        else
            m_hoverFlat = -1;
        return false;
    }

    if (ev.type == skEventType::MouseDown) {
        if (ev.x < x || ev.x >= x + w || ev.y < y || ev.y >= y + h) {
            setVisible(false);
            return false;
        }
        int hit = flatAt(ev.y);
        if (hit >= 0 && !m_items[hit].isSeparator && m_items[hit].enabled) {
            auto action = m_items[hit].action;
            setVisible(false);
            if (action) action();
        }
        return true;
    }

    return false;
}
