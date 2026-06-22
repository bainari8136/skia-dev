#pragma once
#include "skWidget.h"
#include <include/core/SkColor.h>
#include <string>

// Circular avatar showing initials on an auto-tinted background.
class skAvatar : public skWidget {
public:
    // cx, cy = center point; radius = circle radius.
    skAvatar(int cx, int cy, int radius);

    void setInitials(const std::string& initials) { m_initials = initials; }
    void setColor(SkColor bg) { m_bg = bg; m_colorSet = true; }

    void Paint(SkCanvas* canvas) override;

private:
    int         m_radius;
    std::string m_initials;
    SkColor     m_bg       = SK_ColorTRANSPARENT;
    bool        m_colorSet = false;

    SkColor pickColor() const;
};
