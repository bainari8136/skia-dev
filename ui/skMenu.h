#pragma once
#include "skWidget.h"
#include <string>
#include <vector>
#include <functional>
#include <memory>

// Dropdown overlay. Add via skWindow::addOverlay().
// Call openAt() to position and show; handleEvent() closes on outside click or Escape.
class skMenu : public skWidget {
public:
    skMenu();

    static std::shared_ptr<skMenu> make();

    void addItem(const std::string& label, std::function<void()> action, bool enabled = true);
    void addSeparator();

    // Positions the menu below (ox, oy) and makes it visible.
    void openAt(int ox, int oy);

    void Paint(SkCanvas* canvas) override;
    bool handleEvent(const skEvent& ev) override;
    void onMouseLeave() override { m_hoverFlat = -1; }

private:
    std::shared_ptr<skMenu> shared_this() {
        return std::static_pointer_cast<skMenu>(shared_from_this());
    }

    struct Item {
        std::string           label;
        std::function<void()> action;
        bool                  enabled     = true;
        bool                  isSeparator = false;
    };
    std::vector<Item> m_items;
    int m_hoverFlat = -1;

    static constexpr float kItemH = 28.f;
    static constexpr float kSepH  =  9.f;
    static constexpr float kPadX  = 16.f;
    static constexpr float kMinW  = 140.f;

    // Compute (w, h) from items. Called in openAt().
    void computeSize();

    // Returns flat item index for a given client-y, or -1.
    int flatAt(int py) const;
};
