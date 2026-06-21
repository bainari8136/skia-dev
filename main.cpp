#include <Windows.h>
#include "core/skApp.h"
#include "platform/skWindow.h"
#include "ui/skButton.h"
#include "ui/skLabel.h"
#include "ui/skCheckBox.h"
#include "ui/skSlider.h"

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE, LPSTR, int cmdShow) {
    skApp app(hInstance, cmdShow);

    skWindow* window = app.createWindow("skWidgets Demo", 800, 600);
    if (!window) {
        MessageBoxA(nullptr, "Failed to create window.", "skWidgets", MB_ICONERROR | MB_OK);
        return 1;
    }

    HWND hwnd = window->hwnd();

    // Title
    auto title = std::make_shared<skLabel>(50, 50, 700, 44, "skWidgets Demo", 32.0f);
    title->setColor(SkColorSetRGB(30, 30, 30));
    window->addWidget(title);

    // Divider label
    auto divider = std::make_shared<skLabel>(50, 105, 700, 20, "A Skia + Win32 UI framework", 14.0f);
    divider->setColor(SkColorSetRGB(130, 130, 130));
    window->addWidget(divider);

    // Volume slider + live label
    auto volLabel = std::make_shared<skLabel>(50, 160, 400, 22, "Volume: 65%", 15.0f);
    volLabel->setColor(SkColorSetRGB(60, 60, 60));
    window->addWidget(volLabel);

    auto slider = std::make_shared<skSlider>(50, 194, 340, 26);
    slider->setValue(0.65f);
    slider->setOnChange([volLabel, hwnd](float v) {
        int pct = static_cast<int>(v * 100.f);
        volLabel->setText("Volume: " + std::to_string(pct) + "%");
        InvalidateRect(hwnd, nullptr, FALSE);
    });
    window->addWidget(slider);

    // Checkbox
    auto check = std::make_shared<skCheckBox>(50, 258, 280, 28, "Enable notifications");
    check->setChecked(true);
    window->addWidget(check);

    auto check2 = std::make_shared<skCheckBox>(50, 298, 280, 28, "Start on login");
    window->addWidget(check2);

    // Button
    auto btn = std::make_shared<skButton>(50, 370, 130, 38, "Click Me");
    btn->setOnClick([hwnd]() {
        MessageBoxA(hwnd, "Hello from skWidgets!", "Button Clicked", MB_OK);
    });
    window->addWidget(btn);

    return app.run();
}
