#pragma once
#include "skWidget.h"
#include <memory>

// Material-style card: rounded rect with a blurred drop shadow.
class skCard : public skWidget {
public:
    skCard(int x, int y, int w, int h, float radius = 12.f);

    static std::shared_ptr<skCard> make(int x, int y, int w, int h, float radius = 12.f);

    void setElevation(float e) { m_elevation = e; }
    void setRadius(float r)    { m_radius    = r; }

    void Paint(SkCanvas* canvas) override;

    std::shared_ptr<skCard> elevation(float e);
    std::shared_ptr<skCard> radius(float r);
    std::shared_ptr<skCard> pos(int px, int py);
    std::shared_ptr<skCard> size(int pw, int ph);

private:
    std::shared_ptr<skCard> shared_this() {
        return std::static_pointer_cast<skCard>(shared_from_this());
    }

    float m_radius    = 12.f;
    float m_elevation =  4.f;
};
