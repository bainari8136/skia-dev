#pragma once
#include <Windows.h>
#include <vector>
#include <memory>
#include "rendering/skRenderContext.h"
#include "ui/skWidget.h"
#include "ui/skEvent.h"

class skWindow {
public:
    skWindow(const char* title, int width, int height);
    ~skWindow() = default;

    bool create(HINSTANCE hInstance);
    void show(int cmdShow);
    void addWidget(std::shared_ptr<skWidget> widget);
    // Overlays are painted last and get first-pass on click events (can consume them).
    void addOverlay(std::shared_ptr<skWidget> widget);

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

    HWND        m_hwnd      = nullptr;
    HINSTANCE   m_hInstance = nullptr;
    const char* m_title;
    int         m_width;
    int         m_height;

    skRenderContext                        m_ctx;
    std::vector<std::shared_ptr<skWidget>> m_widgets;
    std::vector<std::shared_ptr<skWidget>> m_overlays;
    std::shared_ptr<skWidget>              m_focus;
};
