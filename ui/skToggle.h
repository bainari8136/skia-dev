#pragma once
#include "skWidget.h"
#include <string>
#include <functional>

// iOS-style toggle switch with an animated sliding thumb.
class skToggle : public skWidget {
public:
    skToggle(int x, int y, int w, int h, std::string label = "");

    bool checked()          const { return m_checked; }
    void setChecked(bool v)       { m_checked = v; m_thumbAnim = v ? 1.f : 0.f; }
    void setOnChange(std::function<void(bool)> cb) { m_onChange = std::move(cb); }

    void Paint(SkCanvas* canvas) override;
    void OnEvent(const skEvent& event) override;
    void onTick()                      override;
    void onMouseLeave()                override { m_hovered = false; }

private:
    static constexpr int kPillW = 44;
    static constexpr int kPillH = 24;

    bool        m_checked   = false;
    float       m_thumbAnim = 0.f; // 0=off, 1=on, interpolated
    bool        m_hovered   = false;
    std::string m_label;
    std::function<void(bool)> m_onChange;
};
