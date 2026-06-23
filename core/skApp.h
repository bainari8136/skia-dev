#pragma once
#include <Windows.h>
#include <memory>
#include "platform/skWindow.h"

class skApp {
public:
    skApp(HINSTANCE hInstance, int cmdShow);
    ~skApp();

    skWindow* createWindow(const char* title, int width, int height);
    int run();

private:
    HINSTANCE              m_hInstance;
    int                    m_cmdShow;
    std::unique_ptr<skWindow> m_window;
    bool m_comInitialized = false;
};
