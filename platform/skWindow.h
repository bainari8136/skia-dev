#pragma once
#include <Windows.h>
#include <vector>
#include <memory>
#include <string>
#include <functional>
#include "rendering/skRenderContext.h"
#include "ui/skWidget.h"
#include "ui/skEvent.h"

class skToast;

class skWindow {
public:
    skWindow(const char* title, int width, int height);
    ~skWindow() = default;

    bool create(HINSTANCE hInstance);
    void show(int cmdShow);
    void addWidget(std::shared_ptr<skWidget> widget);
    void addOverlay(std::shared_ptr<skWidget> widget);

    // Show a slide-in / slide-out notification at the bottom of the window.
    void showToast(const std::string& msg, int durationTicks = 25);

    // Register a callback fired on every WM_SIZE with the new client (w, h).
    void setOnResize(std::function<void(int, int)> cb) { m_onResize = std::move(cb); }

    HWND hwnd() const { return m_hwnd; }

private:
    static LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp);
    LRESULT handleMessage(UINT msg, WPARAM wp, LPARAM lp);

    void onSize(int w, int h);
    void onPaint();
    void blitToWindow();
    void dispatchEvent(const skEvent& ev);
    void setFocus(std::shared_ptr<skWidget> w);
    std::shared_ptr<skWidget> findFocusTarget(int mx, int my);
    void cycleFocus(bool reverse);
    void drawTooltip(SkCanvas* canvas, skWidget* w);

    HWND        m_hwnd      = nullptr;
    HINSTANCE   m_hInstance = nullptr;
    const char* m_title;
    int         m_width;
    int         m_height;

    skRenderContext                        m_ctx;
    std::vector<std::shared_ptr<skWidget>> m_widgets;
    std::vector<std::shared_ptr<skWidget>> m_overlays;
    std::shared_ptr<skWidget>              m_focus;

    // Hover tracking for tooltips
    skWidget* m_hoverWidget = nullptr;
    int       m_hoverTicks  = 0;

    std::shared_ptr<skToast>     m_toast;
    void repositionToast();
    std::function<void(int,int)> m_onResize;
};
