#include <Windows.h>
#include "core/skApp.h"
#include "platform/skWindow.h"
#include "ui/skButton.h"

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE, LPSTR, int cmdShow) {
    skApp app(hInstance, cmdShow);

    skWindow* window = app.createWindow("skWidgets Demo", 800, 600);
    if (!window) {
        MessageBoxA(nullptr, "Failed to create window.", "skWidgets", MB_ICONERROR | MB_OK);
        return 1;
    }

    auto btn = std::make_shared<skButton>(340, 282, 120, 36, "Click Me");
    btn->setOnClick([window]() {
        MessageBoxA(window->hwnd(), "Hello from skWidgets!", "Button Clicked", MB_OK);
    });
    window->addWidget(btn);

    return app.run();
}
