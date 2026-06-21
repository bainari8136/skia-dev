#pragma once
#include "skWidget.h"
#include <vector>
#include <memory>

// Scrollable container.  Children use coordinates RELATIVE to the panel
// origin (0, 0) — the panel translates the Skia canvas before painting them
// and adjusts event coordinates before forwarding.
//
// Note: children are NOT in skWindow's widget tree, so they do not receive
// onTick() calls and cannot hold keyboard focus.  Keep children read-only
// or use mouse-only interaction.
class skScrollPanel : public skWidget {
public:
    skScrollPanel(int x, int y, int w, int h);

    void addChild(std::shared_ptr<skWidget> child);

    void Paint(SkCanvas* canvas) override;
    void OnEvent(const skEvent& event) override;

private:
    static constexpr float kSbW = 10.f;

    std::vector<std::shared_ptr<skWidget>> m_children;
    int m_scrollOffset = 0;

    int contentHeight() const;
    int maxScroll()     const;
};
