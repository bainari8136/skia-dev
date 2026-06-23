#include "skSizer.h"

skSizer::skSizer(skDirection dir, int gap) : m_dir(dir), m_gap(gap) {}

std::shared_ptr<skSizer> skSizer::make(skDirection dir, int gap) {
    return std::make_shared<skSizer>(dir, gap);
}

std::shared_ptr<skSizer> skSizer::add(std::shared_ptr<skWidget> widget, int fixedSize) {
    m_children.push_back({ std::move(widget), fixedSize });
    return shared_from_this();
}

std::shared_ptr<skSizer> skSizer::direction(skDirection dir) {
    m_dir = dir;
    return shared_from_this();
}

std::shared_ptr<skSizer> skSizer::gap(int g) {
    m_gap = g;
    return shared_from_this();
}

void skSizer::layout(int startX, int startY, int crossSize) {
    int cursor = (m_dir == skDirection::Column) ? startY : startX;

    for (auto& entry : m_children) {
        auto& w = entry.widget;

        if (m_dir == skDirection::Column) {
            w->x = startX;
            w->y = cursor;
            if (crossSize > 0) w->w = crossSize;
            if (entry.fixedSize > 0) w->h = entry.fixedSize;
            cursor += w->h + m_gap;
        } else {
            w->x = cursor;
            w->y = startY;
            if (crossSize > 0) w->h = crossSize;
            if (entry.fixedSize > 0) w->w = entry.fixedSize;
            cursor += w->w + m_gap;
        }
    }
}
