#pragma once
#include "skWidget.h"
#include <string>
#include <vector>
#include <functional>
#include <memory>

// Vertical navigation list with an active selection indicator.
class skSideBar : public skWidget {
public:
    skSideBar(int cx, int cy, int cw, int ch);

    static std::shared_ptr<skSideBar> make(int cx, int cy, int cw, int ch);

    void addItem(const std::string& label);
    void setSelected(int idx);
    int  selected() const { return m_selected; }

    void setOnChange(std::function<void(int, const std::string&)> fn);

    void Paint(SkCanvas* canvas) override;
    void OnEvent(const skEvent& ev) override;
    void onMouseLeave() override { m_hoverIdx = -1; }

    std::shared_ptr<skSideBar> withSelected(int idx);
    std::shared_ptr<skSideBar> onChange(std::function<void(int, const std::string&)> fn);
    std::shared_ptr<skSideBar> pos(int px, int py);
    std::shared_ptr<skSideBar> size(int pw, int ph);

private:
    std::shared_ptr<skSideBar> shared_this() {
        return std::static_pointer_cast<skSideBar>(shared_from_this());
    }

    std::vector<std::string> m_items;
    int m_selected = 0;
    int m_hoverIdx = -1;
    std::function<void(int, const std::string&)> m_onChange;

    static constexpr float kItemH = 36.f;
    static constexpr float kPadX  = 14.f;
    static constexpr float kBarW  =  3.f;

    int itemAt(int py) const;
};
