#pragma once
#include "skWidget.h"
#include <string>
#include <vector>
#include <memory>

// Floating content panel that appears above an anchor point.
// Add as an overlay via skWindow::addOverlay(). Call openAt(ax, ay) where (ax, ay)
// is the anchor (e.g. button center-top). Closes on outside click or Escape.
class skPopover : public skWidget {
public:
    skPopover(int contentW, int contentH);

    static std::shared_ptr<skPopover> make(int contentW, int contentH);

    // Opens the popover centered horizontally on ax, with its bottom touching ay.
    void openAt(int ax, int ay);

    void setTitle(const std::string& title)  { m_title = title; }
    void addLine(const std::string& text)    { m_lines.push_back(text); }

    void Paint(SkCanvas* canvas) override;
    bool handleEvent(const skEvent& ev) override;

    std::shared_ptr<skPopover> title(const std::string& t);

private:
    std::shared_ptr<skPopover> shared_this() {
        return std::static_pointer_cast<skPopover>(shared_from_this());
    }

    std::string              m_title;
    std::vector<std::string> m_lines;

    static constexpr float kRadius = 8.f;
    static constexpr float kArrow  = 7.f;
    static constexpr float kPadX   = 14.f;
    static constexpr float kPadY   = 10.f;
    static constexpr float kLineH  = 18.f;
};
