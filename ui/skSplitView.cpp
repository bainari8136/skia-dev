#include "skSplitView.h"
#include "skTheme.h"
#include <include/core/SkPaint.h>
#include <algorithm>
#include <cmath>

skSplitView::skSplitView(int sx, int sy, int sw, int sh, float initSplit)
    : skWidget(sx, sy, sw, sh), m_splitPos(initSplit) {}

void skSplitView::setLeft(std::shared_ptr<skWidget> widget) {
    m_left = std::move(widget); updateChildBounds();
}

void skSplitView::setRight(std::shared_ptr<skWidget> widget) {
    m_right = std::move(widget); updateChildBounds();
}

void skSplitView::updateChildBounds() {
    float dx = divX();
    if (m_left) {
        m_left->x = x; m_left->y = y;
        m_left->w = std::max(1, (int)(dx - (float)x));
        m_left->h = h;
    }
    if (m_right) {
        m_right->x = (int)(dx + kDivW); m_right->y = y;
        m_right->w = std::max(1, (int)((float)(x + w) - dx - kDivW));
        m_right->h = h;
    }
}

bool skSplitView::overDivider(int px, int py) const {
    float dx = divX();
    return px >= (int)dx && px < (int)(dx + kDivW) && py >= y && py < y + h;
}

void skSplitView::Paint(SkCanvas* canvas) {
    const auto& th = skGetTheme();

    if (m_left  && m_left->visible())  m_left->Paint(canvas);
    if (m_right && m_right->visible()) m_right->Paint(canvas);

    float dx = divX();
    SkPaint dp; dp.setAntiAlias(true);
    dp.setColor(m_hoverDiv || m_dragging ? th.accent : th.panelBorder);
    canvas->drawRect(SkRect::MakeXYWH(dx, (float)y, kDivW, (float)h), dp);

    // Three drag dots on the divider
    float cx_ = dx + kDivW / 2.f, cy_ = (float)y + (float)h / 2.f;
    SkPaint dotp; dotp.setAntiAlias(true);
    dotp.setColor(m_hoverDiv || m_dragging ? SK_ColorWHITE : th.textSecondary);
    for (int i = -2; i <= 2; ++i)
        canvas->drawCircle(cx_, cy_ + (float)i * 5.f, 1.5f, dotp);
}

void skSplitView::OnEvent(const skEvent& ev) {
    if (ev.type == skEventType::MouseMove) {
        m_hoverDiv = overDivider(ev.x, ev.y) || m_dragging;
        if (m_dragging) {
            float minX = (float)x + kMinW;
            float maxX = (float)(x + w) - kDivW - kMinW;
            float clampedX = std::max(minX, std::min(maxX, (float)ev.x));
            m_splitPos = (clampedX - (float)x) / (float)w;
            updateChildBounds();
            return;
        }
        if (m_left  && m_left->contains(ev.x, ev.y))  m_left->OnEvent(ev);
        if (m_right && m_right->contains(ev.x, ev.y)) m_right->OnEvent(ev);
        return;
    }
    if (ev.type == skEventType::MouseDown) {
        if (overDivider(ev.x, ev.y)) { m_dragging = true; return; }
        if (m_left  && m_left->contains(ev.x, ev.y))  { m_left->OnEvent(ev);  return; }
        if (m_right && m_right->contains(ev.x, ev.y)) { m_right->OnEvent(ev); return; }
        return;
    }
    if (ev.type == skEventType::MouseUp) {
        m_dragging = false;
        if (m_left  && m_left->contains(ev.x, ev.y))  m_left->OnEvent(ev);
        if (m_right && m_right->contains(ev.x, ev.y)) m_right->OnEvent(ev);
        return;
    }
    if (ev.type == skEventType::MouseWheel) {
        if (m_left  && m_left->contains(ev.x, ev.y))  { m_left->OnEvent(ev);  return; }
        if (m_right && m_right->contains(ev.x, ev.y)) { m_right->OnEvent(ev); return; }
    }
}
