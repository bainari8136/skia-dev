#pragma once
#include "skWidget.h"
#include <include/core/SkColor.h>
#include <vector>
#include <memory>

// Container widget that stacks children vertically (or horizontally).
// Owns its children — do NOT add them to skWindow separately.
// Call layout() after adding all children to compute positions and auto-size h.
class skStackPanel : public skWidget {
public:
    skStackPanel(int x, int y, int w, int h = 0);

    void addChild(std::shared_ptr<skWidget> child);
    void setGap    (int g)              { m_gap   = g;  }
    void setPadding(int px, int py)     { m_padX  = px; m_padY = py; }
    void setHorizontal(bool h)          { m_horiz = h;  }
    void setBg(SkColor bg, float r = 0.f) { m_bg = bg; m_radius = r; m_hasBg = true; }
    void setBorder(SkColor c, float w = 1.f) { m_borderCol = c; m_borderW = w; m_hasBorder = true; }

    // Recompute child positions and auto-size this panel to fit children.
    void layout();

    void Paint(SkCanvas* canvas) override;
    void OnEvent(const skEvent& ev) override;

private:
    std::vector<std::shared_ptr<skWidget>> m_children;
    int   m_gap   = 8;
    int   m_padX  = 0;
    int   m_padY  = 0;
    bool  m_horiz = false;

    bool    m_hasBg     = false;
    SkColor m_bg        = SK_ColorTRANSPARENT;
    float   m_radius    = 0.f;

    bool    m_hasBorder = false;
    SkColor m_borderCol = SK_ColorTRANSPARENT;
    float   m_borderW   = 1.f;
};
