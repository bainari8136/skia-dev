#pragma once
#include "skWidget.h"
#include <string>
#include <vector>
#include <functional>

class skTabBar : public skWidget {
public:
    skTabBar(int x, int y, int w, int h);

    void addTab(std::string label);
    int  selectedIndex()      const { return m_selected; }

    void setOnChange(std::function<void(int, const std::string&)> cb) { m_onChange = std::move(cb); }

    void Paint(SkCanvas* canvas) override;
    void OnEvent(const skEvent& event) override;

private:
    std::vector<std::string>                     m_tabs;
    int                                          m_selected = 0;
    int                                          m_hovered  = -1;
    std::function<void(int, const std::string&)> m_onChange;

    float tabWidth() const { return m_tabs.empty() ? 0.f : (float)w / (float)m_tabs.size(); }
    int   hitTab(int mx)  const;
};
