#pragma once
#include <include/core/SkCanvas.h>
#include "skEvent.h"

class skWidget {
public:
    skWidget(int x, int y, int w, int h) : x(x), y(y), w(w), h(h) {}
    virtual ~skWidget() = default;

    virtual void Paint(SkCanvas* canvas) = 0;
    virtual void OnEvent(const skEvent& event) {}
    // Return true to consume the event (stops propagation to lower widgets).
    virtual bool handleEvent(const skEvent& ev) { OnEvent(ev); return false; }

    virtual bool canFocus()       const { return false; }
    virtual void onFocusGained()        {}
    virtual void onFocusLost()          {}

    virtual void onTick()               {} // fired ~every 530 ms by WM_TIMER
    virtual void onMouseEnter()         {}
    virtual void onMouseLeave()         {}

    // Hooks for controls that host native platform content. The opaque host
    // is an HWND on Windows.
    virtual void setNativeHost(void* host) { (void)host; }
    virtual void syncNativeView(bool visible) { (void)visible; }

    void setTooltip(std::string t)      { m_tooltip = std::move(t); }
    const std::string& tooltip() const  { return m_tooltip; }

    void setVisible(bool v) { m_visible = v; }
    bool visible()    const { return m_visible; }

    bool contains(int px, int py) const {
        return px >= x && px < x + w && py >= y && py < y + h;
    }

    int x, y, w, h;

protected:
    std::string m_tooltip;
    bool        m_visible = true;
};
