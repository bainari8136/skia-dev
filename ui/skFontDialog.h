#pragma once
#include "skWidget.h"
#include <string>
#include <vector>
#include <functional>

struct skFontSelection {
    std::string family = "Segoe UI";
    int  size   = 14;
    bool bold   = false;
    bool italic = false;
};

// Font picker overlay using the system font list (Win32 EnumFontFamiliesEx).
// Add via skWindow::addOverlay(). Call show() to open.
class skFontDialog : public skWidget {
public:
    skFontDialog(int winW, int winH);

    void show(skFontSelection initial = {});
    void setOnConfirm(std::function<void(skFontSelection)> fn) { m_onConfirm = std::move(fn); }
    void setOnCancel (std::function<void()>                fn) { m_onCancel  = std::move(fn); }

    void Paint(SkCanvas* canvas) override;
    bool handleEvent(const skEvent& ev) override;

private:
    std::vector<std::string> m_fonts;
    skFontSelection          m_sel;
    int  m_fontScroll  = 0;
    int  m_hoverFont   = -1;
    bool m_okHov = false, m_cancelHov = false;

    std::function<void(skFontSelection)> m_onConfirm;
    std::function<void()>                m_onCancel;

    static constexpr int   kDlgW    = 420;
    static constexpr int   kDlgH    = 360;
    static constexpr float kR       = 10.f;
    static constexpr float kListW   = 220.f;
    static constexpr float kListH   = 220.f;
    static constexpr float kListY   = 42.f;
    static constexpr float kRowH    = 20.f;
    static constexpr float kSbW     = 8.f;
    static constexpr float kBtnW    = 80.f;
    static constexpr float kBtnH    = 30.f;
    static constexpr float kPrevH   = 60.f;

    float dlgX() const { return (float)x + ((float)w - kDlgW) / 2.f; }
    float dlgY() const { return (float)y + ((float)h - kDlgH) / 2.f; }

    SkRect listRect()    const;
    SkRect okRect()      const;
    SkRect cancelRect()  const;
    SkRect previewRect() const;
    SkRect boldRect()    const;
    SkRect italicRect()  const;
    SkRect sizeUpRect()  const;
    SkRect sizeDnRect()  const;

    int  fontAt(int py) const;
    int  visibleFonts() const;
    int  maxFontScroll() const;
    void ensureVisible(int idx);

    void enumFonts();
};
