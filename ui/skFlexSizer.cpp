#include "skFlexSizer.h"
#include <algorithm>

skFlexSizer::skFlexSizer(Direction dir, int gap)
    : m_dir(dir), m_gap(gap) {}

void skFlexSizer::addFixed(std::shared_ptr<skWidget> w, int size) {
    m_entries.push_back({std::move(w), size, 0.f});
}
void skFlexSizer::addFlex(std::shared_ptr<skWidget> w, float flex) {
    m_entries.push_back({std::move(w), -1, flex});
}

void skFlexSizer::layout(int x, int y, int totalW, int totalH) {
    int n = (int)m_entries.size();
    int totalGap = n > 1 ? m_gap * (n - 1) : 0;

    if (m_dir == Direction::Column) {
        int   fixedTotal = 0;
        float flexTotal  = 0.f;
        for (auto& e : m_entries) {
            if (e.fixedSz >= 0) fixedTotal += e.fixedSz;
            else                flexTotal  += e.flex;
        }
        int remaining = totalH - fixedTotal - totalGap;
        int curY = y;
        for (auto& e : m_entries) {
            int sz = e.fixedSz >= 0 ? e.fixedSz
                                    : (flexTotal > 0.f ? (int)((float)remaining * e.flex / flexTotal) : 0);
            e.widget->x = x;  e.widget->y = curY;
            e.widget->w = totalW; e.widget->h = sz;
            curY += sz + m_gap;
        }
    } else { // Row
        int   fixedTotal = 0;
        float flexTotal  = 0.f;
        for (auto& e : m_entries) {
            if (e.fixedSz >= 0) fixedTotal += e.fixedSz;
            else                flexTotal  += e.flex;
        }
        int remaining = totalW - fixedTotal - totalGap;
        int curX = x;
        for (auto& e : m_entries) {
            int sz = e.fixedSz >= 0 ? e.fixedSz
                                    : (flexTotal > 0.f ? (int)((float)remaining * e.flex / flexTotal) : 0);
            e.widget->x = curX; e.widget->y = y;
            e.widget->w = sz;   e.widget->h = totalH;
            curX += sz + m_gap;
        }
    }
}
