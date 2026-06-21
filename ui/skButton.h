#pragma once
#include "skWidget.h"
#include <string>
#include <functional>

class skButton : public skWidget {
public:
    skButton(int x, int y, int w, int h, std::string label = "Button");

    void Paint(SkCanvas* canvas) override;
    void OnEvent(const skEvent& event) override;

    void setOnClick(std::function<void()> cb) { m_onClick = std::move(cb); }
    void setLabel(std::string label)           { m_label  = std::move(label); }
    void onTick()                              override;

private:
    std::string          m_label;
    bool                 m_hovered   = false;
    bool                 m_pressed   = false;
    float                m_hoverAnim = 0.f;  // 0=normal, 1=hovered (animated)
    std::function<void()> m_onClick;
};
