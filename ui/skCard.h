#pragma once
#include "skWidget.h"

// Material-style card: rounded rect with a blurred drop shadow.
class skCard : public skWidget {
public:
    skCard(int x, int y, int w, int h, float radius = 12.f);

    void setElevation(float e) { m_elevation = e; }
    void setRadius(float r)    { m_radius    = r; }

    void Paint(SkCanvas* canvas) override;

private:
    float m_radius    = 12.f;
    float m_elevation =  4.f;
};
