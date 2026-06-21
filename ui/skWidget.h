#pragma once
#include <include/core/SkCanvas.h>
#include "skEvent.h"

class skWidget {
public:
    skWidget(int x, int y, int w, int h) : x(x), y(y), w(w), h(h) {}
    virtual ~skWidget() = default;

    virtual void Paint(SkCanvas* canvas) = 0;
    virtual void OnEvent(const skEvent& event) {}

    virtual bool canFocus()       const { return false; }
    virtual void onFocusGained()        {}
    virtual void onFocusLost()          {}

    bool contains(int px, int py) const {
        return px >= x && px < x + w && py >= y && py < y + h;
    }

    int x, y, w, h;
};
