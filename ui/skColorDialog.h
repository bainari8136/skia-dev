#pragma once
#include "skWidget.h"
#include <functional>

// HSV color-picker overlay.
// Add via skWindow::addOverlay(). Call show(initialColor) to open.
class skColorDialog : public skWidget {
public:
    skColorDialog(int winW, int winH);

    void show(SkColor initialColor = SK_ColorRED);

    void setOnConfirm(std::function<void(SkColor)> fn) { m_onConfirm = std::move(fn); }
    void setOnCancel (std::function<void()>         fn) { m_onCancel  = std::move(fn); }

    void Paint(SkCanvas* canvas) override;
    bool handleEvent(const skEvent& ev) override;

private:
    float m_hue = 0.f;  // 0..360
    float m_sat = 1.f;  // 0..1
    float m_val = 1.f;  // 0..1

    bool m_dragSV    = false;
    bool m_dragHue   = false;
    bool m_okHov     = false;
    bool m_cancelHov = false;

    std::function<void(SkColor)> m_onConfirm;
    std::function<void()>        m_onCancel;

    static constexpr int   kDlgW  = 340;
    static constexpr int   kDlgH  = 300;
    static constexpr float kR     = 10.f;
    static constexpr float kSVW   = 200.f;
    static constexpr float kSVH   = 160.f;
    static constexpr float kHueW  =  18.f;
    static constexpr float kBtnW  =  80.f;
    static constexpr float kBtnH  =  32.f;

    float dlgX() const { return (float)x + ((float)w - kDlgW) / 2.f; }
    float dlgY() const { return (float)y + ((float)h - kDlgH) / 2.f; }

    SkRect svRect()     const;
    SkRect hueRect()    const;
    SkRect okRect()     const;
    SkRect cancelRect() const;

    SkColor currentColor() const;
    static SkColor hsvToRgb(float h, float s, float v);
    void pickSV (int mx, int my);
    void pickHue(int my);
};
