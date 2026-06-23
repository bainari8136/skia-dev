#include "skDockPanel.h"

skDockPanel::skDockPanel(int sx, int sy, int sw, int sh)
    : skWidget(sx, sy, sw, sh) {}

void skDockPanel::addChild(std::shared_ptr<skWidget> child, Dock dock, int size) {
    m_children.push_back({std::move(child), dock, size});
}

void skDockPanel::layout() {
    // Remaining available rect starts at the panel bounds
    int rx = x, ry = y, rw = w, rh = h;

    for (auto& e : m_children) {
        auto& c = e.widget;
        switch (e.dock) {
        case Dock::Left:
            c->x = rx; c->y = ry; c->w = e.size; c->h = rh;
            rx += e.size; rw -= e.size;
            break;
        case Dock::Right:
            c->x = rx + rw - e.size; c->y = ry; c->w = e.size; c->h = rh;
            rw -= e.size;
            break;
        case Dock::Top:
            c->x = rx; c->y = ry; c->w = rw; c->h = e.size;
            ry += e.size; rh -= e.size;
            break;
        case Dock::Bottom:
            c->x = rx; c->y = ry + rh - e.size; c->w = rw; c->h = e.size;
            rh -= e.size;
            break;
        case Dock::Fill:
            c->x = rx; c->y = ry; c->w = rw; c->h = rh;
            break;
        }
    }
}

void skDockPanel::Paint(SkCanvas* canvas) {
    for (auto& e : m_children)
        if (e.widget->visible()) e.widget->Paint(canvas);
}

void skDockPanel::OnEvent(const skEvent& ev) {
    for (auto& e : m_children)
        if (e.widget->visible()) e.widget->OnEvent(ev);
}

void skDockPanel::setNativeHost(void* host) {
    for (auto& e : m_children)
        e.widget->setNativeHost(host);
}

void skDockPanel::syncNativeView(bool visible) {
    for (auto& e : m_children)
        e.widget->syncNativeView(visible && e.widget->visible());
}
