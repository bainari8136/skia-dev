#pragma once
#include "skWidget.h"
#include <string>
#include <vector>
#include <memory>
#include <functional>

struct skTreeNode {
    std::string label;
    bool expanded = true;
    std::vector<std::unique_ptr<skTreeNode>> children;
};

// Hierarchical tree widget.
// Children of a node are visible when expanded = true.
// Left/Right arrow keys collapse/expand; Up/Down navigate.
// Click on the chevron icon to toggle; click on the label to select.
class skTreeView : public skWidget {
public:
    skTreeView(int x, int y, int w, int h);

    skTreeNode* addRoot(const std::string& label);
    skTreeNode* addChild(skTreeNode* parent, const std::string& label);

    void setOnSelect(std::function<void(const std::string&)> cb) { m_onSelect = std::move(cb); }

    bool canFocus()      const override { return true; }
    void onFocusGained()       override { m_focused = true; }
    void onFocusLost()         override { m_focused = false; }
    void onMouseLeave()        override { m_hoverRow = -1; }

    void Paint(SkCanvas* canvas) override;
    void OnEvent(const skEvent& ev) override;

private:
    struct FlatNode { skTreeNode* node; int depth; };

    void buildFlat(std::vector<FlatNode>& out,
                   std::vector<std::unique_ptr<skTreeNode>>& nodes, int depth);
    std::vector<FlatNode> getFlat();

    std::vector<std::unique_ptr<skTreeNode>> m_roots;
    int  m_scrollOffset = 0;
    int  m_selectedRow  = -1;
    int  m_hoverRow     = -1;
    bool m_focused      = false;

    std::function<void(const std::string&)> m_onSelect;

    static constexpr float kRowH    = 26.f;
    static constexpr float kIndentW = 18.f;
    static constexpr float kChevW   = 14.f;

    int visibleRows() const { return std::max(1, (int)((float)h / kRowH)); }
};
