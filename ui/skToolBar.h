#pragma once
#include "skWidget.h"
#include <string>
#include <vector>
#include <functional>
#include <memory>

// Horizontal strip of text buttons with optional separators.
class skToolBar : public skWidget {
public:
    skToolBar(int cx, int cy, int cw, int ch);

    static std::shared_ptr<skToolBar> make(int cx, int cy, int cw, int ch);

    void addItem(const std::string& label, std::function<void()> action);
    void addSeparator();

    void Paint(SkCanvas* canvas) override;
    void OnEvent(const skEvent& ev) override;
    void onMouseLeave() override { m_hoverIdx = -1; }

    std::shared_ptr<skToolBar> pos(int px, int py);
    std::shared_ptr<skToolBar> size(int pw, int ph);

private:
    std::shared_ptr<skToolBar> shared_this() {
        return std::static_pointer_cast<skToolBar>(shared_from_this());
    }

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
