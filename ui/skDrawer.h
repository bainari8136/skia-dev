#pragma once
#include "skWidget.h"
#include <string>
#include <vector>
#include <functional>

// Animated slide-in navigation drawer from the left edge.
// Add as an overlay via skWindow::addOverlay().
// Call open() / close() to show / hide. Closes on outside click or Escape.
class skDrawer : public skWidget {
public:
    // totalW/H = full window size (for backdrop). drawerW = slide-in panel width.
    skDrawer(int totalW, int totalH, int drawerW = 260);

    static std::shared_ptr<skDrawer> make(int totalW, int totalH, int drawerW = 260);

    void setTitle(const std::string& title) { m_title = title; }
    void addItem(const std::string& label, std::function<void()> action = nullptr);

    void open();
    void close();
    bool isOpen() const { return m_open; }

    void Paint(SkCanvas* canvas) override;
    bool handleEvent(const skEvent& ev) override;
    void onTick() override;

    std::shared_ptr<skDrawer> title(const std::string& t);

private:
    std::shared_ptr<skDrawer> shared_this() {
        return std::static_pointer_cast<skDrawer>(shared_from_this());
    }

    int   m_drawerW;
    float m_offset = 0.f; // 0 = closed, 1 = open
    bool  m_open   = false;

    struct Item { std::string label; std::function<void()> action; };
    std::string       m_title;
    std::vector<Item> m_items;
    int m_hoverIdx = -1;

    static constexpr float kItemH  = 44.f;
    static constexpr float kTitleH = 52.f;

    float panelLeft() const { return -(float)m_drawerW * (1.f - m_offset); }
    int   itemAt(int px, int py) const;
};
