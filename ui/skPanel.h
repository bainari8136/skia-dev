#pragma once
#include "skWidget.h"
#include <include/core/SkColor.h>

class skPanel : public skWidget {
public:
    skPanel(int x, int y, int w, int h);
    void Paint(SkCanvas* canvas) override;

    void setBackground(SkColor c)   { m_bg     = c; }
    void setBorder(SkColor c)        { m_border = c; }
    void setRadius(float r)          { m_radius = r; }

private:
    SkColor m_bg     = SK_ColorWHITE;
    SkColor m_border = SkColorSetRGB(218, 220, 228);
    float   m_radius = 10.f;
};
