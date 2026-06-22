#include <Windows.h>
#include "skStackPanel.h"
#include <include/core/SkPaint.h>
#include <include/core/SkRRect.h>

skStackPanel::skStackPanel(int sx, int sy, int sw, int sh)
    : skWidget(sx, sy, sw, sh) {}

void skStackPanel::addChild(std::shared_ptr<skWidget> child) {
    m_children.push_back(std::move(child));
}

void skStackPanel::layout() {
    if (m_horiz) {
        int curX = x + m_padX;
        int innerH = h - 2 * m_padY;
        for (auto& c : m_children) {
            c->x = curX; c->y = y + m_padY;
            c->h = (innerH > 0 ? innerH : c->h);
            curX += c->w + m_gap;
        }
        // Auto-width not supported for horizontal (caller controls w)
    } else {
        int curY = y + m_padY;
        int innerW = w - 2 * m_padX;
        for (auto& c : m_children) {
            c->x = x + m_padX; c->y = curY;
            c->w = (innerW > 0 ? innerW : c->w);
            curY += c->h + m_gap;
        }
        if (!m_children.empty()) curY -= m_gap;
        curY += m_padY;
        h = curY - y;
    }
}

void skStackPanel::Paint(SkCanvas* canvas) {
    if (m_hasBg) {
        SkRRect rr; rr.setRectXY(SkRect::MakeXYWH((float)x,(float)y,(float)w,(float)h), m_radius, m_radius);
        SkPaint bgP; bgP.setAntiAlias(true); bgP.setColor(m_bg);
        canvas->drawRRect(rr, bgP);
    }
    for (auto& c : m_children)
        if (c->visible()) c->Paint(canvas);
    if (m_hasBorder) {
        SkRRect rr; rr.setRectXY(SkRect::MakeXYWH((float)x,(float)y,(float)w,(float)h), m_radius, m_radius);
        SkPaint brd; brd.setAntiAlias(true);
        brd.setStyle(SkPaint::kStroke_Style); brd.setStrokeWidth(m_borderW);
        brd.setColor(m_borderCol);
        canvas->drawRRect(rr, brd);
    }
}

void skStackPanel::OnEvent(const skEvent& ev) {
    for (auto& c : m_children)
        if (c->visible()) c->OnEvent(ev);
}
