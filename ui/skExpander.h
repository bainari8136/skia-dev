#pragma once
#include "skWidget.h"
#include <string>
#include <vector>
#include <memory>

// Collapsible section with animated expand/collapse.
// Children are positioned in local coordinates (0,0 = expander top-left).
// Content area begins at y = headerH inside the widget.
// The widget's total h must cover headerH + contentH.
class skExpander : public skWidget {
public:
    skExpander(int x, int y, int w, int totalH, int headerH, std::string title);

    void addChild(std::shared_ptr<skWidget> child);
    void setExpanded(bool v);
    bool expanded() const { return m_expanded; }

    void Paint(SkCanvas* canvas) override;
    void OnEvent(const skEvent& ev) override;
    void onTick() override;

private:
    std::string m_title;
    int         m_headerH;
    bool        m_expanded  = true;
    bool        m_hovered   = false;
    float       m_openAnim  = 1.f; // 0 = collapsed, 1 = fully open

    std::vector<std::shared_ptr<skWidget>> m_children;

    int contentH() const { return h - m_headerH; }
};
