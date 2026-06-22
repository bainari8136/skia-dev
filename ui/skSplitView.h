#pragma once
#include "skWidget.h"
#include <memory>

// Two horizontally-split panes separated by a draggable divider.
// Children are owned by the SplitView; do NOT add them to skWindow separately.
class skSplitView : public skWidget {
public:
    skSplitView(int x, int y, int w, int h, float initSplit = 0.5f);

    void setLeft(std::shared_ptr<skWidget> widget);
    void setRight(std::shared_ptr<skWidget> widget);

    float splitPos() const { return m_splitPos; }

    void Paint(SkCanvas* canvas) override;
    void OnEvent(const skEvent& ev) override;
    void onMouseLeave() override { m_dragging = false; m_hoverDiv = false; }

private:
    float m_splitPos;
    bool  m_dragging = false;
    bool  m_hoverDiv = false;

    std::shared_ptr<skWidget> m_left;
    std::shared_ptr<skWidget> m_right;

    static constexpr float kDivW = 5.f;
    static constexpr float kMinW = 50.f;

    float divX() const { return (float)x + m_splitPos * (float)w; }
    void  updateChildBounds();
    bool  overDivider(int px, int py) const;
};
