#pragma once
#include "skWidget.h"
#include <string>
#include <include/core/SkColor.h>

// Pill-shaped label badge.  Auto-sizes to fit its text.
// Defaults to the theme accent colour; call setColor() to override.
class skBadge : public skWidget {
public:
    skBadge(int x, int y, std::string text, float fontSize = 11.f);

    void setText(std::string t)               { m_text = std::move(t); }
    void setColor(SkColor bg, SkColor text)   { m_bg = bg; m_fg = text; m_colorSet = true; }

    void Paint(SkCanvas* canvas) override;

private:
    std::string m_text;
    float       m_fontSize;
    SkColor     m_bg       = 0;
    SkColor     m_fg       = SK_ColorWHITE;
    bool        m_colorSet = false;
};
