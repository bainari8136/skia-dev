#pragma once
#include "skWidget.h"
#include <functional>

// Custom-paint drawing surface.
// Delegates Paint to a user-provided callback; forwards mouse events similarly.
// The callback receives a canvas already translated so (0,0) is the top-left of the widget.
class skCanvasView : public skWidget {
public:
    skCanvasView(int x, int y, int w, int h);

    void setOnPaint    (std::function<void(SkCanvas*, int w, int h)>        fn);
    void setOnMouseDown(std::function<void(int localX, int localY)>         fn);
    void setOnMouseMove(std::function<void(int localX, int localY, bool drag)> fn);
    void setOnMouseUp  (std::function<void(int localX, int localY)>         fn);

    bool isDragging() const { return m_dragging; }

    void Paint(SkCanvas* canvas) override;
    void OnEvent(const skEvent& ev) override;
    void onMouseLeave() override { m_dragging = false; }

private:
    std::function<void(SkCanvas*,int,int)>    m_onPaint;
    std::function<void(int,int)>              m_onDown;
    std::function<void(int,int,bool)>         m_onMove;
    std::function<void(int,int)>              m_onUp;
    bool m_dragging = false;
};
