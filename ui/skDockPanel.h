#pragma once
#include "skWidget.h"
#include <vector>
#include <memory>

// Layout container that docks children to edges of a rect.
// Children are added in order: each consumes from the remaining rect.
// The last Fill child (or any Fill child) expands to fill remaining space.
// Owns its children — do NOT add them to skWindow separately.
class skDockPanel : public skWidget {
public:
    enum class Dock { Left, Right, Top, Bottom, Fill };

    skDockPanel(int x, int y, int w, int h);

    void addChild(std::shared_ptr<skWidget> child, Dock dock, int size = 0);

    // Recompute all child positions from current x/y/w/h.
    void layout();

    void Paint(SkCanvas* canvas) override;
    void OnEvent(const skEvent& ev) override;

private:
    struct Entry {
        std::shared_ptr<skWidget> widget;
        Dock  dock;
        int   size; // width for Left/Right, height for Top/Bottom; ignored for Fill
    };
    std::vector<Entry> m_children;
};
