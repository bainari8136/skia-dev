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

bool skScrollPanel::thumbGeometry(int ch, float& outThumbH, float& outThumbY) const {
    int ms = std::max(0, ch - h);
    if (ms == 0) return false;
    float ratio  = (float)h / (float)ch;
    outThumbH    = std::max(kMinThumbH, (float)h * ratio);
    float track  = (float)h - outThumbH;
    outThumbY    = (float)y + track * ((float)m_scrollOffset / (float)ms);
    return true;
}

void skScrollPanel::Paint(SkCanvas* canvas) {
    const auto& th = skGetTheme();

    int   ch      = contentHeight();
    int   ms      = std::max(0, ch - h);
    bool  needsSb = ms > 0;
    float viewW   = (float)w - (needsSb ? kSbW : 0.f);

    // Panel background
    SkRRect panelRR;
    panelRR.setRectXY(SkRect::MakeXYWH((float)x, (float)y, (float)w, (float)h), 6.f, 6.f);
    SkPaint bgP;
    bgP.setAntiAlias(true);
    bgP.setColor(th.panelBg);
    canvas->drawRRect(panelRR, bgP);

    // Children — clipped to the viewport (excluding scrollbar strip)
    canvas->save();
    canvas->clipRect(SkRect::MakeLTRB((float)x, (float)y, (float)x + viewW, (float)(y + h)));
    canvas->translate((float)x, (float)y - (float)m_scrollOffset);
    for (auto& c : m_children)
        if (c->visible()) c->Paint(canvas);
    canvas->restore();

    // Scrollbar
    if (needsSb) {
        float sbX = (float)(x + w) - kSbW;

        // Track
        SkPaint sbBg;
        sbBg.setColor(th.trackBg);
        canvas->drawRect(SkRect::MakeXYWH(sbX, (float)y, kSbW, (float)h), sbBg);

        // Thumb
        float thumbH, thumbY;
        thumbGeometry(ch, thumbH, thumbY);

        float tInset = 1.f;
        SkRRect tRR;
        tRR.setRectXY(
            SkRect::MakeXYWH(sbX + tInset, thumbY + tInset, kSbW - tInset*2.f, thumbH - tInset*2.f),
            3.f, 3.f);
        SkPaint sbTh;
        sbTh.setAntiAlias(true);
        sbTh.setColor(m_sbDragging ? th.accent : th.inputBorder);
        canvas->drawRRect(tRR, sbTh);
    }

    // Border (drawn last, on top of everything)
    SkPaint brd;
    brd.setAntiAlias(true);
    brd.setStyle(SkPaint::kStroke_Style);
    brd.setStrokeWidth(1.f);
    brd.setColor(th.inputBorder);
    canvas->drawRRect(panelRR, brd);
}

void skScrollPanel::OnEvent(const skEvent& ev) {
    // --- Mouse wheel: always handle if hovering ---
    if (ev.type == skEventType::MouseWheel) {
        if (contains(ev.x, ev.y)) {
            int delta = (ev.button > 0) ? -kScrollStep : kScrollStep;
            m_scrollOffset = std::max(0, std::min(maxScroll(), m_scrollOffset + delta));
        }
        return;
    }

    // --- Scrollbar drag ---
    float sbX = (float)(x + w) - kSbW;
    bool needsSb = maxScroll() > 0;
    bool inSbStrip = needsSb && (ev.x >= (int)sbX) && contains(ev.x, ev.y);

    if (ev.type == skEventType::MouseDown && inSbStrip) {
        int ch = contentHeight();
        float thumbH, thumbY;
        if (thumbGeometry(ch, thumbH, thumbY)) {
            m_sbDragging      = true;
            m_dragStartY      = ev.y;
            m_dragStartScroll = m_scrollOffset;
            m_dragThumbH      = thumbH;
        }
        return;
    }

    if (ev.type == skEventType::MouseMove && m_sbDragging) {
        int ch    = contentHeight();
        int ms    = std::max(0, ch - h);
        float track = (float)h - m_dragThumbH;
        if (track > 0.f) {
            int delta   = ev.y - m_dragStartY;
            int newScroll = m_dragStartScroll + (int)((float)delta / track * (float)ms);
            m_scrollOffset = std::max(0, std::min(ms, newScroll));
        }
        return;
    }

    if ((ev.type == skEventType::MouseUp || ev.type == skEventType::MouseCancel) && m_sbDragging) {
        m_sbDragging = false;
        return;
    }

    // --- Content-area mouse events ---
    if (ev.type == skEventType::MouseMove ||
        ev.type == skEventType::MouseDown ||
        ev.type == skEventType::MouseUp) {

        skEvent childEv = ev;
        bool inContent = contains(ev.x, ev.y) && !inSbStrip;

        if (inContent) {
            childEv.x = ev.x - x;
            childEv.y = ev.y - y + m_scrollOffset;
        } else {
            // Out-of-bounds sentinel — clears hover states in children
            childEv.x = -1;
            childEv.y = -1;
        }
        for (auto& c : m_children)
            if (c->visible()) c->OnEvent(childEv);
    }
}
