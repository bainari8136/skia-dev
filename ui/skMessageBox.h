#pragma once
#include "skWidget.h"
#include <string>
#include <functional>

enum class skMessageType    { Info, Warning, Error, Success };
enum class skMessageButtons { OK, OKCancel, YesNo };

// M3-styled alert dialog overlay. Add via skWindow::addOverlay().
// Call show() to display; closes on OK/Confirm, Escape/Cancel, or Enter.
class skMessageBox : public skWidget {
public:
    skMessageBox(int winW, int winH);

    void show(const std::string& title,
              const std::string& message,
              skMessageType      type    = skMessageType::Info,
              skMessageButtons   buttons = skMessageButtons::OK);

    void setOnConfirm(std::function<void()> fn) { m_onConfirm = fn; }
    void setOnCancel (std::function<void()> fn) { m_onCancel  = fn; }
    void setOnClose  (std::function<void()> fn) { m_onConfirm = fn; } // compat

    void Paint(SkCanvas* canvas) override;
    bool handleEvent(const skEvent& ev) override;

private:
    std::string      m_title;
    std::string      m_message;
    skMessageType    m_type    = skMessageType::Info;
    skMessageButtons m_buttons = skMessageButtons::OK;

    bool m_confirmHover = false;
    bool m_cancelHover  = false;

    // M3 "extra-large" shape, 480dp wide dialog
    static constexpr int   kDlgW   = 480;
    static constexpr int   kDlgH   = 248;
    static constexpr float kRadius = 28.f;  // M3 extra-large corner shape
    static constexpr float kPad    = 24.f;  // M3 dialog padding
    static constexpr float kIconR  = 20.f;  // icon container circle radius
    static constexpr float kBtnW   = 104.f;
    static constexpr float kBtnH   = 40.f;  // M3 button height
    static constexpr float kBtnR   = 20.f;  // fully rounded M3 button

    float dlgX() const { return (float)(x + (w - kDlgW) / 2); }
    float dlgY() const { return (float)(y + (h - kDlgH) / 2); }

    float confirmBtnX() const { return dlgX() + (float)kDlgW - kPad - kBtnW; }
    float confirmBtnY() const { return dlgY() + (float)kDlgH - kPad - kBtnH; }
    float cancelBtnX()  const { return confirmBtnX() - kBtnW - 8.f; }
    float cancelBtnY()  const { return confirmBtnY(); }

    bool inConfirm(int px, int py) const;
    bool inCancel (int px, int py) const;
    bool hasCancel() const { return m_buttons != skMessageButtons::OK; }

    SkColor     typeColor()    const;
    void        drawIcon      (SkCanvas* canvas, float cx, float cy) const;
    const char* confirmLabel() const;
    const char* cancelLabel()  const;

    std::function<void()> m_onConfirm;
    std::function<void()> m_onCancel;
};
