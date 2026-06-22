#pragma once
#include "skWidget.h"
#include <string>
#include <functional>

// Standalone tooltip overlay. Add via skWindow::addOverlay().
// Call showAt() to display near a screen position; it auto-dismisses
// after durationTicks × 100 ms (0 = stay until hide() is called).
class skTooltip : public skWidget {
public:
    skTooltip(int winW, int winH);

    void showAt(int anchorX, int anchorY, const std::string& text, int durationTicks = 20);
    void hide();

    void Paint(SkCanvas* canvas) override;
    bool handleEvent(const skEvent& ev) override;
    void onTick() override;

private:
    std::string m_text;
    int  m_anchorX    = 0;
    int  m_anchorY    = 0;
    int  m_ticksLeft  = 0; // 0 = infinite
    int  m_maxTicks   = 0;
    bool m_pinned     = false; // true when durationTicks==0

    static constexpr int   kPadX   = 10;
    static constexpr int   kPadY   = 6;
    static constexpr float kR      = 5.f;
    static constexpr int   kOffset = 14; // below anchor
};
