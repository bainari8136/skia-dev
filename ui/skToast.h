#pragma once
#include "skWidget.h"
#include <string>
#include <memory>

// Slide-in / slide-out notification banner anchored to the window bottom.
// Managed by skWindow::showToast() — do not add directly as a regular widget.
class skToast : public skWidget {
public:
    skToast(int x, int y, int w, int h);

    static std::shared_ptr<skToast> make(int x, int y, int w, int h);

    // Show the toast.  durationTicks * 100 ms = visible time (excl. slide).
    void show(const std::string& msg, int durationTicks = 25);

    void onTick()               override;
    void Paint(SkCanvas* canvas) override;

    std::shared_ptr<skToast> pos(int px, int py);
    std::shared_ptr<skToast> size(int pw, int ph);

private:
    std::shared_ptr<skToast> shared_this() {
        return std::static_pointer_cast<skToast>(shared_from_this());
    }

    enum class State { Idle, SlideIn, Hold, SlideOut };

    std::string m_msg;
    State       m_state       = State::Idle;
    float       m_slideOffset = 0.f; // extra +y applied during slide (0 = resting)
    int         m_countdown   = 0;
};
