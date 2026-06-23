#pragma once
#include "skWidget.h"
#include <functional>
#include <memory>

class skSlider : public skWidget {
public:
    skSlider(int x, int y, int w, int h, float min = 0.f, float max = 1.f);

    static std::shared_ptr<skSlider> make(int x, int y, int w, int h,
                                           float min = 0.f, float max = 1.f);

    void Paint(SkCanvas* canvas) override;
    void OnEvent(const skEvent& event) override;

    float value() const { return m_value; }
    void  setValue(float v);
    void  setOnChange(std::function<void(float)> cb) { m_onChange = std::move(cb); }

    std::shared_ptr<skSlider> withValue(float v);
    std::shared_ptr<skSlider> onChange(std::function<void(float)> cb);
    std::shared_ptr<skSlider> pos(int px, int py);
    std::shared_ptr<skSlider> size(int pw, int ph);

private:
    std::shared_ptr<skSlider> shared_this() {
        return std::static_pointer_cast<skSlider>(shared_from_this());
    }

    float m_min, m_max, m_value;
    bool  m_dragging = false;
    std::function<void(float)> m_onChange;

    static constexpr float kThumbR  = 10.f;
    static constexpr float kTrackH  =  6.f;

    float thumbCx() const;
    void  updateFromX(int mouseX);
};
