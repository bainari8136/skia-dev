#pragma once
#include "skWidget.h"
#include <functional>

struct skDate { int year = 2024; int month = 1; int day = 1; };

// Calendar-style date picker overlay.
// Add via skWindow::addOverlay(). Call show() to open.
class skDatePicker : public skWidget {
public:
    skDatePicker(int winW, int winH);

    void show(skDate initialDate = {});
    void setOnConfirm(std::function<void(skDate)> fn) { m_onConfirm = std::move(fn); }
    void setOnCancel (std::function<void()>        fn) { m_onCancel  = std::move(fn); }

    void Paint(SkCanvas* canvas) override;
    bool handleEvent(const skEvent& ev) override;

private:
    skDate m_view;   // month/year being displayed
    skDate m_sel;    // selected date
    skDate m_today;  // today (for highlighting)

    bool m_okHov     = false;
    bool m_cancelHov = false;
    bool m_prevHov   = false;
    bool m_nextHov   = false;

    std::function<void(skDate)> m_onConfirm;
    std::function<void()>       m_onCancel;

    static constexpr int   kDlgW    = 280;
    static constexpr int   kDlgH    = 290;
    static constexpr float kR       = 10.f;
    static constexpr float kHeaderH = 42.f;
    static constexpr float kWeekH   = 22.f;
    static constexpr float kCellW   = 36.f;
    static constexpr float kCellH   = 30.f;
    static constexpr float kBtnW    = 72.f;
    static constexpr float kBtnH    = 30.f;

    float dlgX() const { return (float)x + ((float)w - kDlgW) / 2.f; }
    float dlgY() const { return (float)y + ((float)h - kDlgH) / 2.f; }
    float gridX() const { return dlgX() + ((float)kDlgW - 7.f*kCellW) / 2.f; }
    float gridY() const { return dlgY() + kHeaderH + kWeekH + 4.f; }

    SkRect okRect()     const;
    SkRect cancelRect() const;
    SkRect prevRect()   const;
    SkRect nextRect()   const;

    // Returns 1-based day at pixel, 0 if not on a valid day
    int dayAt(int px, int py) const;

    static int  daysInMonth(int year, int month);
    static int  firstWeekday(int year, int month); // 0=Sun
    static const char* monthName(int month);

    void prevMonth();
    void nextMonth();
};
