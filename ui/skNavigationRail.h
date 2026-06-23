#pragma once
#include "skWidget.h"
#include <string>
#include <vector>
#include <functional>

// Vertical navigation rail with icon glyphs and text labels.
// Icons are short UTF-8 strings (Unicode symbols / emoji).
class skNavigationRail : public skWidget {
public:
    skNavigationRail(int x, int y, int w, int h);

    static std::shared_ptr<skNavigationRail> make(int x, int y, int w, int h);

    void addItem(const std::string& icon, const std::string& label,
                 std::function<void()> action = nullptr);
    void setSelected(int idx);
    int  selected() const { return m_selected; }

    void setOnChange(std::function<void(int, const std::string&)> fn);

    void Paint(SkCanvas* canvas) override;
    void OnEvent(const skEvent& ev) override;
    void onMouseLeave() override { m_hoverIdx = -1; }

    std::shared_ptr<skNavigationRail> withSelected(int idx);
    std::shared_ptr<skNavigationRail> onChange(std::function<void(int, const std::string&)> fn);
    std::shared_ptr<skNavigationRail> pos(int px, int py);
    std::shared_ptr<skNavigationRail> size(int pw, int ph);

private:
    std::shared_ptr<skNavigationRail> shared_this() {
        return std::static_pointer_cast<skNavigationRail>(shared_from_this());
    }

    struct Item {
        std::string icon;
        std::string label;
        std::function<void()> action;
    };
    std::vector<Item> m_items;
    int m_selected = 0;
    int m_hoverIdx = -1;
    std::function<void(int, const std::string&)> m_onChange;

    static constexpr float kItemH  = 64.f;
    static constexpr float kIconSz = 18.f;
    static constexpr float kBarW   =  3.f;

    int itemAt(int py) const;
};
