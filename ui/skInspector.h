#pragma once
#include "skWidget.h"
#include <string>

// Live theme token inspector. Shows every color token from skGetTheme()
// as a row with: swatch · name · hex value.
// Auto-refreshes on onTick() so theme changes are reflected immediately.
class skInspector : public skWidget {
public:
    skInspector(int x, int y, int w, int h);

    void Paint(SkCanvas* canvas) override;
    void OnEvent(const skEvent& ev) override;
    void onTick() override { /* triggers repaint via skWindow invalidation */ }

private:
    int m_scroll = 0;
    int m_hover  = -1;

    static constexpr int kRowH  = 24;
    static constexpr int kSbW   = 8;
    static constexpr int kSwSz  = 16;
    static constexpr int kPad   = 8;
};
