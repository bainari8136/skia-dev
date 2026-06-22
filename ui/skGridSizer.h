#pragma once
#include "skWidget.h"
#include <vector>
#include <memory>

// N-column grid layout tool. Not a widget — use like skSizer:
// call layout(), then window->addWidget() each child.
class skGridSizer {
public:
    explicit skGridSizer(int cols, int gapX = 8, int gapY = 8);

    void add(std::shared_ptr<skWidget> widget, int height);
    void layout(int startX, int startY, int totalW);

    struct Entry { std::shared_ptr<skWidget> widget; int h; };
    const std::vector<Entry>& children() const { return m_entries; }

private:
    int m_cols, m_gapX, m_gapY;
    std::vector<Entry> m_entries;
};
