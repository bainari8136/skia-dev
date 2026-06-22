#include "skGridSizer.h"
#include <algorithm>

skGridSizer::skGridSizer(int cols, int gapX, int gapY)
    : m_cols(cols), m_gapX(gapX), m_gapY(gapY) {}

void skGridSizer::add(std::shared_ptr<skWidget> widget, int height) {
    m_entries.push_back({std::move(widget), height});
}

void skGridSizer::layout(int startX, int startY, int totalW) {
    if (m_cols <= 0 || m_entries.empty()) return;
    int cellW = (totalW - m_gapX * (m_cols - 1)) / m_cols;

    int i = 0, rowY = startY;
    while (i < (int)m_entries.size()) {
        // Row height = max height among this row's entries
        int rowH = 0;
        for (int j = i; j < i + m_cols && j < (int)m_entries.size(); ++j)
            rowH = std::max(rowH, m_entries[j].h);

        for (int col = 0; col < m_cols && i + col < (int)m_entries.size(); ++col) {
            auto& e = m_entries[i + col];
            e.widget->x = startX + col * (cellW + m_gapX);
            e.widget->y = rowY;
            e.widget->w = cellW;
            e.widget->h = e.h;
        }

        rowY += rowH + m_gapY;
        i    += m_cols;
    }
}
