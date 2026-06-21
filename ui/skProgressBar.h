#pragma once
#include "skWidget.h"
#include <algorithm>

class skProgressBar : public skWidget {
public:
    skProgressBar(int x, int y, int w, int h);
    void Paint(SkCanvas* canvas) override;

    float value() const { return m_value; }
    void  setValue(float v) { m_value = std::max(0.f, std::min(1.f, v)); }

private:
    float m_value = 0.f;
};
