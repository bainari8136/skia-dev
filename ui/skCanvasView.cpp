#include <Windows.h>
#include "skCanvasView.h"
#include "skTheme.h"
#include <include/core/SkPaint.h>
#include <include/core/SkRRect.h>

skCanvasView::skCanvasView(int sx, int sy, int sw, int sh)
    : skWidget(sx, sy, sw, sh) {}

void skCanvasView::setOnPaint    (std::function<void(SkCanvas*,int,int)>    fn) { m_onPaint = std::move(fn); }
void skCanvasView::setOnMouseDown(std::function<void(int,int)>              fn) { m_onDown  = std::move(fn); }
void skCanvasView::setOnMouseMove(std::function<void(int,int,bool)>         fn) { m_onMove  = std::move(fn); }
void skCanvasView::setOnMouseUp  (std::function<void(int,int)>              fn) { m_onUp    = std::move(fn); }

void skCanvasView::Paint(SkCanvas* canvas) {
    const auto& th = skGetTheme();

    // Border + clip
    SkRRect rr; rr.setRectXY(SkRect::MakeXYWH((float)x,(float)y,(float)w,(float)h), 4.f, 4.f);
    SkPaint bgP; bgP.setAntiAlias(true); bgP.setColor(th.inputBg);
    canvas->drawRRect(rr, bgP);

    canvas->save();
    canvas->clipRRect(rr, SkClipOp::kIntersect, true);
    canvas->translate((float)x, (float)y);

    if (m_onPaint) m_onPaint(canvas, w, h);

    canvas->restore();

    SkPaint brd; brd.setAntiAlias(true);
    brd.setStyle(SkPaint::kStroke_Style); brd.setStrokeWidth(1.f);
    brd.setColor(th.inputBorder);
    canvas->drawRRect(rr, brd);
}

void skCanvasView::OnEvent(const skEvent& ev) {
    int lx = ev.x - x, ly = ev.y - y;
    switch (ev.type) {
    case skEventType::MouseDown:
        if (!contains(ev.x, ev.y)) break;
        m_dragging = true;
        if (m_onDown) m_onDown(lx, ly);
        break;
    case skEventType::MouseMove:
        if (m_onMove) m_onMove(lx, ly, m_dragging);
        break;
    case skEventType::MouseUp:
        m_dragging = false;
        if (m_onUp) m_onUp(lx, ly);
        break;
    default: break;
    }
}
