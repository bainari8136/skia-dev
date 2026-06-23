#include "platform/skWindow.h"
#include "ui/skTheme.h"
#include "ui/skTypeface.h"
#include "ui/skToast.h"
#include <include/core/SkFont.h>
#include <include/core/SkPaint.h>
#include <include/core/SkRRect.h>
#include <windowsx.h>

static const char* const SK_WNDCLASS = "skWidgetWindow";

skWindow::skWindow(const char* title, int width, int height)
    : m_title(title), m_width(width), m_height(height) {}

bool skWindow::create(HINSTANCE hInstance) {
    m_hInstance = hInstance;

    WNDCLASSEX wc   = {};
    wc.cbSize        = sizeof(WNDCLASSEX);
    wc.lpfnWndProc   = WndProc;
    wc.hInstance     = hInstance;
    wc.lpszClassName = SK_WNDCLASS;
    wc.hCursor       = LoadCursor(nullptr, IDC_ARROW);
    wc.hbrBackground = nullptr;

    if (!RegisterClassEx(&wc)) return false;

    m_hwnd = CreateWindowEx(
        WS_EX_CLIENTEDGE, SK_WNDCLASS, m_title,
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT, m_width, m_height,
        nullptr, nullptr, hInstance,
        this
    );

    return m_hwnd != nullptr;
}

void skWindow::show(int cmdShow) {
    ShowWindow(m_hwnd, cmdShow);
    UpdateWindow(m_hwnd);
}

void skWindow::addWidget(std::shared_ptr<skWidget> widget) {
    widget->setNativeHost(m_hwnd);
    m_widgets.push_back(std::move(widget));
    if (m_hwnd) InvalidateRect(m_hwnd, nullptr, FALSE);
}

void skWindow::addOverlay(std::shared_ptr<skWidget> widget) {
    widget->setNativeHost(m_hwnd);
    m_overlays.push_back(std::move(widget));
    if (m_hwnd) InvalidateRect(m_hwnd, nullptr, FALSE);
}

// ---------------------------------------------------------------------------
// Focus
// ---------------------------------------------------------------------------

std::shared_ptr<skWidget> skWindow::findFocusTarget(int mx, int my) {
    for (auto& w : m_widgets) {
        if (w->visible() && w->canFocus() && w->contains(mx, my))
            return w;
    }
    return nullptr;
}

void skWindow::setFocus(std::shared_ptr<skWidget> w) {
    if (w == m_focus) return;
    if (m_focus) m_focus->onFocusLost();
    m_focus = w;
    if (m_focus) m_focus->onFocusGained();
    InvalidateRect(m_hwnd, nullptr, FALSE);
}

void skWindow::cycleFocus(bool reverse) {
    std::vector<std::shared_ptr<skWidget>> focusables;
    for (auto& w : m_widgets)
        if (w->visible() && w->canFocus()) focusables.push_back(w);
    if (focusables.empty()) return;

    int idx = -1;
    for (int i = 0; i < (int)focusables.size(); ++i)
        if (focusables[i] == m_focus) { idx = i; break; }

    int next;
    if (reverse)
        next = (idx <= 0) ? (int)focusables.size() - 1 : idx - 1;
    else
        next = (idx + 1) % (int)focusables.size();

    setFocus(focusables[next]);
}

// ---------------------------------------------------------------------------
// Rendering
// ---------------------------------------------------------------------------

void skWindow::repositionToast() {
    if (!m_toast) return;
    m_toast->x = (m_width  - m_toast->w) / 2;
    m_toast->y =  m_height - m_toast->h - 20;
}

void skWindow::onSize(int w, int h) {
    m_width  = w;
    m_height = h;
    m_ctx.resize(w, h);
    repositionToast();
    if (m_onResize) m_onResize(w, h);
    InvalidateRect(m_hwnd, nullptr, FALSE);
}

void skWindow::showToast(const std::string& msg, int durationTicks) {
    if (m_toast) m_toast->show(msg, durationTicks);
    InvalidateRect(m_hwnd, nullptr, FALSE);
}

void skWindow::onPaint() {
    m_ctx.clear(skGetTheme().windowBg);
    SkCanvas* canvas = m_ctx.getCanvas();
    if (!canvas) return;

    for (auto& widget : m_widgets) {
        widget->syncNativeView(widget->visible());
        if (widget->visible()) widget->Paint(canvas);
    }
    for (auto& widget : m_overlays) {
        widget->syncNativeView(widget->visible());
        if (widget->visible()) widget->Paint(canvas);
    }

    if (m_hoverWidget && m_hoverTicks >= 5 && !m_hoverWidget->tooltip().empty())
        drawTooltip(canvas, m_hoverWidget);

    blitToWindow();
}

void skWindow::blitToWindow() {
    SkPixmap pixmap;
    if (!m_ctx.readPixels(pixmap)) return;

    BITMAPINFO bmi            = {};
    bmi.bmiHeader.biSize      = sizeof(BITMAPINFOHEADER);
    bmi.bmiHeader.biWidth     = m_width;
    bmi.bmiHeader.biHeight    = -m_height;
    bmi.bmiHeader.biPlanes    = 1;
    bmi.bmiHeader.biBitCount  = 32;
    bmi.bmiHeader.biCompression = BI_RGB;

    PAINTSTRUCT ps;
    HDC hdc   = BeginPaint(m_hwnd, &ps);
    HDC memdc = CreateCompatibleDC(hdc);

    void*   bits = nullptr;
    HBITMAP hbmp = CreateDIBSection(hdc, &bmi, DIB_RGB_COLORS, &bits, nullptr, 0);
    HBITMAP hold = (HBITMAP)SelectObject(memdc, hbmp);

    memcpy(bits, pixmap.addr(), (size_t)m_width * m_height * 4);
    BitBlt(hdc, 0, 0, m_width, m_height, memdc, 0, 0, SRCCOPY);

    SelectObject(memdc, hold);
    DeleteObject(hbmp);
    DeleteDC(memdc);
    EndPaint(m_hwnd, &ps);
}

// ---------------------------------------------------------------------------
// Event dispatch
// ---------------------------------------------------------------------------

void skWindow::dispatchEvent(const skEvent& ev) {
    // Give overlays (menus, modals, popovers) first crack at every event.
    // handleEvent() returns true to consume; the default base just calls OnEvent().
    bool consumed = false;
    for (auto& w : m_overlays) {
        if (w->visible() && w->handleEvent(ev)) { consumed = true; break; }
    }

    if (!consumed) {
        for (auto& widget : m_widgets) if (widget->visible()) widget->OnEvent(ev);
    }
    InvalidateRect(m_hwnd, nullptr, FALSE);
}

void skWindow::drawTooltip(SkCanvas* canvas, skWidget* w) {
    const auto& th = skGetTheme();
    static sk_sp<SkTypeface> s_tf = skGetSystemTypeface();
    SkFont font(s_tf, 12.f);
    font.setEdging(SkFont::Edging::kAntiAlias);

    const std::string& tip = w->tooltip();
    SkRect bounds;
    font.measureText(tip.c_str(), tip.size(), SkTextEncoding::kUTF8, &bounds);

    const float pad = 8.f;
    float tipW = bounds.width() + pad * 2.f;
    float tipH = bounds.height() + pad * 2.f;

    float tx = (float)w->x;
    float ty = (float)(w->y + w->h) + 4.f;
    if (tx + tipW > (float)m_width)  tx = (float)m_width - tipW - 4.f;
    if (ty + tipH > (float)m_height) ty = (float)w->y - tipH - 4.f;

    SkRRect rr;
    rr.setRectXY(SkRect::MakeXYWH(tx, ty, tipW, tipH), 4.f, 4.f);

    SkPaint bg;
    bg.setAntiAlias(true);
    bg.setColor(th.panelBg);
    canvas->drawRRect(rr, bg);

    SkPaint brd;
    brd.setAntiAlias(true);
    brd.setStyle(SkPaint::kStroke_Style);
    brd.setStrokeWidth(1.f);
    brd.setColor(th.panelBorder);
    canvas->drawRRect(rr, brd);

    SkPaint tp;
    tp.setAntiAlias(true);
    tp.setColor(th.textPrimary);
    canvas->drawString(tip.c_str(), tx + pad - bounds.left(), ty + pad - bounds.top(), font, tp);
}

// ---------------------------------------------------------------------------
// Win32 message handling
// ---------------------------------------------------------------------------

LRESULT CALLBACK skWindow::WndProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp) {
    skWindow* self = nullptr;

    if (msg == WM_NCCREATE) {
        auto* cs = reinterpret_cast<CREATESTRUCT*>(lp);
        self     = reinterpret_cast<skWindow*>(cs->lpCreateParams);
        SetWindowLongPtr(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(self));
        self->m_hwnd = hwnd;
    } else {
        self = reinterpret_cast<skWindow*>(GetWindowLongPtr(hwnd, GWLP_USERDATA));
    }

    if (self) return self->handleMessage(msg, wp, lp);
    return DefWindowProc(hwnd, msg, wp, lp);
}

LRESULT skWindow::handleMessage(UINT msg, WPARAM wp, LPARAM lp) {
    switch (msg) {
        case WM_CREATE:
            m_ctx.resize(m_width, m_height);
            SetTimer(m_hwnd, 1, 100, nullptr);
            m_toast = std::make_shared<skToast>(0, 0, 320, 48);
            m_toast->setVisible(false);
            m_overlays.push_back(m_toast);
            repositionToast();
            return 0;

        case WM_SIZE:
            if (LOWORD(lp) > 0 && HIWORD(lp) > 0)
                onSize(LOWORD(lp), HIWORD(lp));
            return 0;

        case WM_PAINT:
            onPaint();
            return 0;

        case WM_ERASEBKGND:
            return 1;

        case WM_TIMER:
            for (auto& w : m_widgets) if (w->visible()) w->onTick();
            for (auto& w : m_overlays) if (w->visible()) w->onTick();
            if (m_hoverWidget && !m_hoverWidget->tooltip().empty()) ++m_hoverTicks;
            InvalidateRect(m_hwnd, nullptr, FALSE);
            return 0;

        case WM_MOUSEWHEEL: {
            POINT pt{ GET_X_LPARAM(lp), GET_Y_LPARAM(lp) };
            ScreenToClient(m_hwnd, &pt);
            skEvent wev;
            wev.type   = skEventType::MouseWheel;
            wev.x      = pt.x;
            wev.y      = pt.y;
            wev.button = GET_WHEEL_DELTA_WPARAM(wp);
            for (auto& widget : m_widgets) {
                if (widget->contains(wev.x, wev.y)) {
                    widget->OnEvent(wev);
                    InvalidateRect(m_hwnd, nullptr, FALSE);
                    break;
                }
            }
            return 0;
        }

        case WM_MOUSEMOVE: {
            int mx = GET_X_LPARAM(lp), my = GET_Y_LPARAM(lp);
            // Update hover tracking for tooltip delay
            skWidget* hit = nullptr;
            for (auto it = m_widgets.rbegin(); it != m_widgets.rend(); ++it) {
                if ((*it)->visible() && (*it)->contains(mx, my)) { hit = it->get(); break; }
            }
            if (hit != m_hoverWidget) {
                if (m_hoverWidget) m_hoverWidget->onMouseLeave();
                m_hoverWidget = hit;
                m_hoverTicks  = 0;
                if (m_hoverWidget) m_hoverWidget->onMouseEnter();
            }
            dispatchEvent({ skEventType::MouseMove, mx, my });
            return 0;
        }

        case WM_LBUTTONDOWN: {
            SetCapture(m_hwnd);
            int mx = GET_X_LPARAM(lp), my = GET_Y_LPARAM(lp);
            setFocus(findFocusTarget(mx, my));
            dispatchEvent({ skEventType::MouseDown, mx, my });
            return 0;
        }

        case WM_LBUTTONUP:
            dispatchEvent({ skEventType::MouseUp, GET_X_LPARAM(lp), GET_Y_LPARAM(lp) });
            ReleaseCapture();
            return 0;

        case WM_CANCELMODE:
            // Windows is cancelling the active mouse interaction. Release capture and
            // notify widgets so no drag state can remain latched.
            dispatchEvent({ skEventType::MouseCancel });
            ReleaseCapture();
            return 0;

        case WM_CAPTURECHANGED:
            // Capture can be taken by another window without a matching button-up.
            dispatchEvent({ skEventType::MouseCancel });
            return 0;

        case WM_CHAR:
            if (m_focus) {
                skEvent e;
                e.type = skEventType::KeyChar;
                e.ch   = static_cast<wchar_t>(wp);
                m_focus->OnEvent(e);
                InvalidateRect(m_hwnd, nullptr, FALSE);
            }
            return 0;

        case WM_KEYDOWN:
            if (wp == VK_TAB) {
                bool shift = (GetKeyState(VK_SHIFT) & 0x8000) != 0;
                cycleFocus(shift);
                return 0;
            }
            {
                skEvent e;
                e.type   = skEventType::KeyDown;
                e.button = static_cast<int>(wp);
                // Overlays (menus, modals) intercept Escape and other keys first.
                bool consumed = false;
                for (auto& w : m_overlays)
                    if (w->visible() && w->handleEvent(e)) { consumed = true; break; }
                if (!consumed && m_focus) m_focus->OnEvent(e);
                InvalidateRect(m_hwnd, nullptr, FALSE);
            }
            return 0;

        case WM_DESTROY:
            KillTimer(m_hwnd, 1);
            PostQuitMessage(0);
            return 0;

        default:
            return DefWindowProc(m_hwnd, msg, wp, lp);
    }
}
