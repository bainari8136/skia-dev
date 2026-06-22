#pragma once
#include "skWidget.h"
#include <string>
#include <vector>

// Simple bar chart widget. Add bars with addBar(label, value); auto-scales to max.
class skChartView : public skWidget {
public:
    skChartView(int x, int y, int w, int h);

    void addBar(const std::string& label, float value);
    void setTitle(const std::string& title) { m_title = title; }
    void clear() { m_bars.clear(); m_hoverIdx = -1; }

    void Paint(SkCanvas* canvas) override;
    void OnEvent(const skEvent& ev) override;
    void onMouseLeave() override { m_hoverIdx = -1; }

private:
    struct Bar { std::string label; float value; };
    std::vector<Bar> m_bars;
    std::string      m_title;
    int              m_hoverIdx = -1;

    static constexpr float kPadL = 38.f;
    static constexpr float kPadB = 30.f;
    static constexpr float kPadT = 26.f;
    static constexpr float kPadR = 10.f;

    float chartW() const { return (float)w - kPadL - kPadR; }
    float chartH() const { return (float)h - kPadT - kPadB; }
    float maxVal()  const;
    int   barAt(int px, int py) const;
};
