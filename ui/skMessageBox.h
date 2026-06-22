#pragma once
#include "skWidget.h"
#include <string>
#include <functional>

enum class skMessageType { Info, Warning, Error };

// Pre-built message dialog overlay. Add via skWindow::addOverlay().
// Call show() to display; closes on OK click or Escape.
class skMessageBox : public skWidget {
public:
    skMessageBox(int winW, int winH);

    void show(const std::string& title, const std::string& message,
              skMessageType type = skMessageType::Info);

    void setOnClose(std::function<void()> fn) { m_onClose = fn; }

    void Paint(SkCanvas* canvas) override;
    bool handleEvent(const skEvent& ev) override;

private:
    std::string   m_title;
    std::string   m_message;
    skMessageType m_type = skMessageType::Info;
    bool          m_okHover = false;

    static constexpr int   kDlgW   = 400;
    static constexpr int   kDlgH   = 200;
    static constexpr float kRadius = 10.f;
    static constexpr float kBarH   =  6.f;
    static constexpr float kBtnW   = 84.f;
    static constexpr float kBtnH   = 34.f;

    int dlgX() const { return x + (w - kDlgW) / 2; }
    int dlgY() const { return y + (h - kDlgH) / 2; }
    int btnX() const { return dlgX() + (kDlgW - (int)kBtnW) / 2; }
    int btnY() const { return dlgY() + kDlgH - (int)kBtnH - 16; }

    SkColor     typeColor()  const;
    const char* typeLabel()  const;

    std::function<void()> m_onClose;
};
