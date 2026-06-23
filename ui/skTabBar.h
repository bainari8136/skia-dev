#pragma once
#include "skWidget.h"
#include <string>
#include <vector>
#include <functional>
#include <memory>

class skTabBar : public skWidget {
public:
    skTabBar(int x, int y, int w, int h);

    static std::shared_ptr<skTabBar> make(int x, int y, int w, int h);

    void addTab(std::string label);
    int  selectedIndex()      const { return m_selected; }

    void setOnChange(std::function<void(int, const std::string&)> cb) { m_onChange = std::move(cb); }

    void Paint(SkCanvas* canvas) override;
    void OnEvent(const skEvent& event) override;

    std::shared_ptr<skTabBar> onChange(std::function<void(int, const std::string&)> cb);
    std::shared_ptr<skTabBar> pos(int px, int py);
    std::shared_ptr<skTabBar> size(int pw, int ph);

private:
    std::shared_ptr<skTabBar> shared_this() {
        return std::static_pointer_cast<skTabBar>(shared_from_this());
    }

    std::vector<std::string>                     m_tabs;
    int                                          m_selected = 0;
    int                                          m_hovered  = -1;
    std::function<void(int, const std::string&)> m_onChange;

    float tabWidth() const { return m_tabs.empty() ? 0.f : (float)w / (float)m_tabs.size(); }
    int   hitTab(int mx)  const;
};
