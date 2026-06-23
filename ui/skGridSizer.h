#pragma once
#include "skWidget.h"
#include <vector>
#include <memory>

// N-column grid layout tool. Not a widget — use like skSizer:
// call layout(), then window->addWidget() each child.
class skGridSizer : public std::enable_shared_from_this<skGridSizer> {
public:
    explicit skGridSizer(int cols, int gapX = 8, int gapY = 8);

    static std::shared_ptr<skGridSizer> make(int cols, int gapX = 8, int gapY = 8);

    // Returns shared_ptr<skGridSizer> for chaining; existing void call sites still compile.
    std::shared_ptr<skGridSizer> add(std::shared_ptr<skWidget> widget, int height);
    void layout(int startX, int startY, int totalW);

    struct Entry { std::shared_ptr<skWidget> widget; int h; };
    const std::vector<Entry>& children() const { return m_entries; }

    // Fluent configuration setters
    std::shared_ptr<skGridSizer> cols(int c);
    std::shared_ptr<skGridSizer> gapX(int g);
    std::shared_ptr<skGridSizer> gapY(int g);

private:
    int m_cols, m_gapX, m_gapY;
    std::vector<Entry> m_entries;
};
