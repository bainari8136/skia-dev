#pragma once
#include "skWidget.h"
#include <vector>
#include <memory>

// Proportional / fixed layout tool. Not a widget — use like skSizer.
// Items are either a fixed size or a flex weight; flex items share remaining space.
class skFlexSizer : public std::enable_shared_from_this<skFlexSizer> {
public:
    enum class Direction { Row, Column };

    explicit skFlexSizer(Direction dir = Direction::Column, int gap = 8);

    static std::shared_ptr<skFlexSizer> make(Direction dir = Direction::Column, int gap = 8);

    // Return shared_ptr<skFlexSizer> for chaining; existing void call sites still compile.
    std::shared_ptr<skFlexSizer> addFixed(std::shared_ptr<skWidget> w, int size);
    std::shared_ptr<skFlexSizer> addFlex (std::shared_ptr<skWidget> w, float flex = 1.f);

    // Distribute layout within the given rect.
    void layout(int x, int y, int totalW, int totalH);

    struct Entry {
        std::shared_ptr<skWidget> widget;
        int   fixedSz; // -1 = flex
        float flex;
    };
    const std::vector<Entry>& children() const { return m_entries; }

    // Fluent configuration setters
    std::shared_ptr<skFlexSizer> direction(Direction dir);
    std::shared_ptr<skFlexSizer> gap(int g);

private:
    Direction          m_dir;
    int                m_gap;
    std::vector<Entry> m_entries;
};
