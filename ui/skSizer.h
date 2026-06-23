#pragma once
#include "skWidget.h"
#include <vector>
#include <memory>

enum class skDirection { Row, Column };

class skSizer : public std::enable_shared_from_this<skSizer> {
public:
    struct Entry {
        std::shared_ptr<skWidget> widget;
        int fixedSize;
    };

    explicit skSizer(skDirection dir = skDirection::Column, int gap = 8);

    static std::shared_ptr<skSizer> make(skDirection dir = skDirection::Column, int gap = 8);

    // Returns shared_ptr<skSizer> for chaining; existing void call sites still compile.
    std::shared_ptr<skSizer> add(std::shared_ptr<skWidget> widget, int fixedSize = 0);

    // Compute child positions starting at (x, y).
    // crossSize > 0 stretches children to fill the cross axis.
    void layout(int x, int y, int crossSize = 0);

    const std::vector<Entry>& children() const { return m_children; }

    // Fluent configuration setters
    std::shared_ptr<skSizer> direction(skDirection dir);
    std::shared_ptr<skSizer> gap(int g);

private:
    skDirection        m_dir;
    int                m_gap;
    std::vector<Entry> m_children;
};
