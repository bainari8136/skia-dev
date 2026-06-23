#pragma once
#include "skWidget.h"
#include <string>
#include <vector>

// Simple bar chart widget. Add bars with addBar(label, value); auto-scales to max.
class skChartView : public skWidget {
public:
    skChartView(int x, int y, int w, int h);

    static std::shared_ptr<skChartView> make(int x, int y, int w, int h);

    void addBar(const std::string& label, float value);
    void setTitle(const std::string& title) { m_title = title; }
    void clear() { m_bars.clear(); m_hoverIdx = -1; }

    void Paint(SkCanvas* canvas) override;
    void OnEvent(const skEvent& ev) override;
    void onMouseLeave() override { m_hoverIdx = -1; }

    std::shared_ptr<skChartView> title(const std::string& t);
    std::shared_ptr<skChartView> pos(int px, int py);
    std::shared_ptr<skChartView> size(int pw, int ph);

private:
    std::shared_ptr<skChartView> shared_this() {
        return std::static_pointer_cast<skChartView>(shared_from_this());
    }

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
