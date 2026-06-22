#pragma once
#include "skWidget.h"
#include <string>
#include <vector>
#include <functional>

// "Home › Section › Page" navigation path.
// All crumbs except the last are clickable links.
class skBreadcrumb : public skWidget {
public:
    skBreadcrumb(int cx, int cy, int cw, int ch);

    void addCrumb(const std::string& label);
    void clear();

    // Fired when a non-last crumb is clicked.
    void setOnClick(std::function<void(int index, const std::string& label)> fn);

    void Paint(SkCanvas* canvas) override;
    void OnEvent(const skEvent& ev) override;
    void onMouseLeave() override { m_hoverIdx = -1; }

private:
    struct Crumb {
        std::string label;
        float       crumbX = 0.f;
        float       crumbW = 0.f;
    };
    std::vector<Crumb> m_crumbs;
    int                m_hoverIdx = -1;
    std::function<void(int, const std::string&)> m_onClick;

    static constexpr float kFontSize = 13.f;
    static constexpr float kSepW     = 18.f;

    void reLayout();
    int  hitTest(int px) const;
};
