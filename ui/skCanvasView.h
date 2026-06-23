#pragma once
#include "skWidget.h"
#include <functional>

// Custom-paint drawing surface.
// Delegates Paint to a user-provided callback; forwards mouse events similarly.
// The callback receives a canvas already translated so (0,0) is the top-left of the widget.
class skCanvasView : public skWidget {
public:
    skCanvasView(int x, int y, int w, int h);

    static std::shared_ptr<skCanvasView> make(int x, int y, int w, int h);

    void setOnPaint    (std::function<void(SkCanvas*, int w, int h)>        fn);
    void setOnMouseDown(std::function<void(int localX, int localY)>         fn);
    void setOnMouseMove(std::function<void(int localX, int localY, bool drag)> fn);
    void setOnMouseUp  (std::function<void(int localX, int localY)>         fn);

    bool isDragging() const { return m_dragging; }

    void Paint(SkCanvas* canvas) override;
    void OnEvent(const skEvent& ev) override;
    void onMouseLeave() override { m_dragging = false; }

    std::shared_ptr<skCanvasView> onPaint(std::function<void(SkCanvas*, int, int)> fn);
    std::shared_ptr<skCanvasView> onMouseDown(std::function<void(int, int)> fn);
    std::shared_ptr<skCanvasView> onMouseMove(std::function<void(int, int, bool)> fn);
    std::shared_ptr<skCanvasView> onMouseUp(std::function<void(int, int)> fn);
    std::shared_ptr<skCanvasView> pos(int px, int py);
    std::shared_ptr<skCanvasView> size(int pw, int ph);

private:
    std::shared_ptr<skCanvasView> shared_this() {
        return std::static_pointer_cast<skCanvasView>(shared_from_this());
    }

    std::function<void(SkCanvas*,int,int)>    m_onPaint;
    std::function<void(int,int)>              m_onDown;
    std::function<void(int,int,bool)>         m_onMove;
    std::function<void(int,int)>              m_onUp;
    bool m_dragging = false;
};
