#pragma once
#include "skWidget.h"
#include <functional>

struct skDate { int year = 2024; int month = 1; int day = 1; };

enum class skDPMode { Calendar, YearPicker };

// Material Design 3 modal date picker overlay.
// Add via skWindow::addOverlay(). Call show() to open.
// Nav-row label is clickable: toggles between calendar and year-picker views.
class skDatePicker : public skWidget {
public:
    skDatePicker(int winW, int winH);

    void show(skDate initialDate = {});
    void setOnConfirm(std::function<void(skDate)> fn) { m_onConfirm = std::move(fn); }
    void setOnCancel (std::function<void()>        fn) { m_onCancel  = std::move(fn); }

    void Paint(SkCanvas* canvas) override;
    bool handleEvent(const skEvent& ev) override;

private:
    skDate   m_view;   // month/year being displayed in the nav row
    skDate   m_sel;    // currently selected date
    skDate   m_today;

    skDPMode m_mode     = skDPMode::Calendar;
    int      m_yearPage = 2020;  // first year on current year-grid page

    bool m_okHov     = false;
    bool m_cancelHov = false;
    bool m_prevHov   = false;
    bool m_nextHov   = false;
    int  m_hovDay    = 0;   // 1-based day under cursor (calendar mode)
    int  m_hovYear   = 0;   // year under cursor (year-picker mode)
    bool m_labelHov  = false;

    std::function<void(skDate)> m_onConfirm;
    std::function<void()>       m_onCancel;

    // M3 dimensions (dp ≈ px at 96 DPI)
    static constexpr int   kDlgW       = 328;
    static constexpr int   kDlgH       = 512;
    static constexpr float kR          = 28.f;
    static constexpr float kHdrH       = 120.f;
    static constexpr float kNavH       = 52.f;
    static constexpr float kWkH        = 40.f;
    static constexpr float kCellW      = 40.f;
    static constexpr float kCellH      = 40.f;
    static constexpr float kSelR       = 20.f;
    static constexpr float kActH       = 60.f;

    // Year-picker grid
    static constexpr int   kYearCols   = 3;
    static constexpr float kYearCellW  = 88.f;
    static constexpr float kYearCellH  = 52.f;
    static constexpr int   kYearsPerPg = 15;   // 3 cols × 5 rows

    float dlgX()     const { return (float)x + ((float)w - kDlgW) * 0.5f; }
    float dlgY()     const { return (float)y + ((float)h - kDlgH) * 0.5f; }
    float gridX()    const { return dlgX() + ((float)kDlgW - 7.f * kCellW) * 0.5f; }
    float gridY()    const { return dlgY() + kHdrH + kNavH + kWkH; }
    float yearGridX() const { return dlgX() + ((float)kDlgW - kYearCols * kYearCellW) * 0.5f; }
    float yearGridY() const { return dlgY() + kHdrH + kNavH + 10.f; }

    SkRect okRect()         const;
    SkRect cancelRect()     const;
    SkRect prevRect()       const;
    SkRect nextRect()       const;
    SkRect monthYearRect()  const;   // clickable nav label

    int dayAt (int px, int py) const;  // 1-based day, 0 = none
    int yearAt(int px, int py) const;  // year, 0 = none

    static int         daysInMonth (int year, int month);
    static int         firstWeekday(int year, int month);
    static const char* monthName   (int month);

    void prevPage();  // prev month (calendar) or prev year-page (year-picker)
    void nextPage();

    void drawChevron  (SkCanvas*, float cx, float cy, bool left, SkColor) const;
    void drawDropArrow(SkCanvas*, float cx, float cy, bool up,   SkColor) const;
};
