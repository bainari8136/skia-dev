#pragma once
#include "skWidget.h"
#include <string>
#include <vector>
#include <functional>
#include <memory>

// "Home › Section › Page" navigation path.
// All crumbs except the last are clickable links.
class skBreadcrumb : public skWidget {
public:
    skBreadcrumb(int cx, int cy, int cw, int ch);

    static std::shared_ptr<skBreadcrumb> make(int cx, int cy, int cw, int ch);

    void addCrumb(const std::string& label);
    void clear();

    // Fired when a non-last crumb is clicked.
    void setOnClick(std::function<void(int index, const std::string& label)> fn);

    void Paint(SkCanvas* canvas) override;
    void OnEvent(const skEvent& ev) override;
    void onMouseLeave() override { m_hoverIdx = -1; }

    std::shared_ptr<skBreadcrumb> onClick(std::function<void(int, const std::string&)> fn);
    std::shared_ptr<skBreadcrumb> pos(int px, int py);
    std::shared_ptr<skBreadcrumb> size(int pw, int ph);

private:
    std::shared_ptr<skBreadcrumb> shared_this() {
        return std::static_pointer_cast<skBreadcrumb>(shared_from_this());
    }

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
