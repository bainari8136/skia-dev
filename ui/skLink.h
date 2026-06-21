#pragma once
#include "skWidget.h"
#include <string>
#include <functional>

// Clickable inline text link — accent colour, underline on hover.
class skLink : public skWidget {
public:
    skLink(int x, int y, std::string text, float fontSize = 13.f);

    void setOnClick(std::function<void()> cb) { m_onClick = std::move(cb); }
    void Paint(SkCanvas* canvas) override;
    void OnEvent(const skEvent& event) override;

private:
    std::string           m_text;
    float                 m_fontSize;
    bool                  m_hovered = false;
    std::function<void()> m_onClick;
};
