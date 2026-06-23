#include "core/skApp.h"
#include <objbase.h>

std::unique_ptr<skApp> skApp::make(HINSTANCE hInstance, int cmdShow) {
    return std::make_unique<skApp>(hInstance, cmdShow);
}

skApp::skApp(HINSTANCE hInstance, int cmdShow)
    : m_hInstance(hInstance), m_cmdShow(cmdShow) {
    m_comInitialized = SUCCEEDED(CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED));
}

skApp::~skApp() {
    m_window.reset();
    if (m_comInitialized) CoUninitialize();
}

skWindow* skApp::createWindow(const char* title, int width, int height) {
    m_window = std::make_unique<skWindow>(title, width, height);
    if (!m_window->create(m_hInstance)) {
        m_window.reset();
        return nullptr;
    }
    m_window->show(m_cmdShow);
    return m_window.get();
}

int skApp::run() {
    MSG msg = {};
    while (GetMessage(&msg, nullptr, 0, 0) > 0) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    return static_cast<int>(msg.wParam);
}
