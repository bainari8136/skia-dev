#pragma once
#include "skWidget.h"
#include <string>
#include <vector>
#include <functional>

// Horizontal strip of text buttons with optional separators.
class skToolBar : public skWidget {
public:
    skToolBar(int cx, int cy, int cw, int ch);

    void addItem(const std::string& label, std::function<void()> action);
    void addSeparator();

    void Paint(SkCanvas* canvas) override;
    void OnEvent(const skEvent& ev) override;
    void onMouseLeave() override { m_hoverIdx = -1; }

private:
    struct Item {
        std::string           label;
        std::function<void()> action;
        bool                  isSep  = false;
        float                 btnX   = 0.f;
        float                 btnW   = 0.f;
    };
    std::vector<Item> m_items;
    int m_hoverIdx = -1;

    static constexpr float kPadX  = 12.f;
    static constexpr float kSepMX =  5.f;

    void reLayout();
    int  hitTest(int px) const;
};
