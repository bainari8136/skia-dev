#include "skScrollPanel.h"
#include "skTheme.h"
#include <include/core/SkPaint.h>
#include <include/core/SkRRect.h>
#include <algorithm>

skScrollPanel::skScrollPanel(int sx, int sy, int sw, int sh)
    : skWidget(sx, sy, sw, sh) {}

void skScrollPanel::addChild(std::shared_ptr<skWidget> child) {
    m_children.push_back(std::move(child));
}

int skScrollPanel::contentHeight() const {
    int maxH = h;
    for (auto& c : m_children) maxH = std::max(maxH, c->y + c->h);
    return maxH;
}

int skScrollPanel::maxScroll() const {
    return std::max(0, contentHeight() - h);
}

void skScrollPanel::Paint(SkCanvas* canvas) {
    const auto& th = skGetTheme();
    const bool  needsSb = maxScroll() > 0;
    const float viewW   = (float)w - (needsSb ? kSbW : 0.f);

    // Panel background
    SkRRect panelRR;
    panelRR.setRectXY(SkRect::MakeXYWH((float)x, (float)y, (float)w, (float)h), 6.f, 6.f);
    SkPaint bgP;
    bgP.setAntiAlias(true);
    bgP.setColor(th.panelBg);
    canvas->drawRRect(panelRR, bgP);

    // Paint children, clipped to the viewport (excluding scrollbar strip)
    canvas->save();
    canvas->clipRect(SkRect::MakeLTRB((float)x, (float)y, (float)x + viewW, (float)(y + h)));
    canvas->translate((float)x, (float)y - (float)m_scrollOffset);
    for (auto& c : m_children)
        if (c->visible()) c->Paint(canvas);
    canvas->restore();

    // Scrollbar
    if (needsSb) {
        float sbX = (float)(x + w) - kSbW;
        SkPaint sbBg;
        sbBg.setColor(th.trackBg);
        canvas->drawRect(SkRect::MakeXYWH(sbX, (float)y, kSbW, (float)h), sbBg);

        int ms = maxScroll();
        float thumbRatio = (float)h / (float)contentHeight();
        float thumbH  = (float)h * thumbRatio;
        float thumbY  = (float)y + ((float)h - thumbH) * ((float)m_scrollOffset / (float)ms);

        SkPaint sbTh;
        sbTh.setAntiAlias(true);
        sbTh.setColor(th.inputBorder);
        SkRRect tRR;
        tRR.setRectXY(SkRect::MakeXYWH(sbX + 1.f, thumbY + 2.f, kSbW - 2.f, thumbH - 4.f), 3.f, 3.f);
        canvas->drawRRect(tRR, sbTh);
    }

    // Border on top
    SkPaint brd;
    brd.setAntiAlias(true);
    brd.setStyle(SkPaint::kStroke_Style);
    brd.setStrokeWidth(1.f);
    brd.setColor(th.inputBorder);
    canvas->drawRRect(panelRR, brd);
}

void skScrollPanel::OnEvent(const skEvent& ev) {
    if (ev.type == skEventType::MouseWheel) {
        if (contains(ev.x, ev.y)) {
            int delta = (ev.button > 0) ? -30 : 30;
            m_scrollOffset = std::max(0, std::min(maxScroll(), m_scrollOffset + delta));
        }
        return;
    }

    // Translate mouse events into content-local coordinates and forward
    if (ev.type == skEventType::MouseMove ||
        ev.type == skEventType::MouseDown ||
        ev.type == skEventType::MouseUp) {
        skEvent childEv = ev;
        if (contains(ev.x, ev.y)) {
            childEv.x = ev.x - x;
            childEv.y = ev.y - y + m_scrollOffset;
        } else {
            childEv.x = -1; childEv.y = -1; // out-of-bounds → clears hover states
        }
        for (auto& c : m_children) if (c->visible()) c->OnEvent(childEv);
    }
}
