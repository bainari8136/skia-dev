#pragma once
#include "skWidget.h"
#include <string>

// Slide-in / slide-out notification banner anchored to the window bottom.
// Managed by skWindow::showToast() — do not add directly as a regular widget.
class skToast : public skWidget {
public:
    skToast(int x, int y, int w, int h);

    // Show the toast.  durationTicks * 100 ms = visible time (excl. slide).
    void show(const std::string& msg, int durationTicks = 25);

    void onTick()               override;
    void Paint(SkCanvas* canvas) override;

private:
    enum class State { Idle, SlideIn, Hold, SlideOut };

    std::string m_msg;
    State       m_state       = State::Idle;
    float       m_slideOffset = 0.f; // extra +y applied during slide (0 = resting)
    int         m_countdown   = 0;
};
