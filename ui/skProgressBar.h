#pragma once
#include "skWidget.h"
#include <algorithm>
#include <memory>

class skProgressBar : public skWidget {
public:
    skProgressBar(int x, int y, int w, int h);

    static std::shared_ptr<skProgressBar> make(int x, int y, int w, int h);

    void Paint(SkCanvas* canvas) override;

    float value() const { return m_value; }
    void  setValue(float v) { m_value = std::max(0.f, std::min(1.f, v)); }

    std::shared_ptr<skProgressBar> withValue(float v);
    std::shared_ptr<skProgressBar> pos(int px, int py);
    std::shared_ptr<skProgressBar> size(int pw, int ph);

private:
    std::shared_ptr<skProgressBar> shared_this() {
        return std::static_pointer_cast<skProgressBar>(shared_from_this());
    }

    float m_value = 0.f;
};
