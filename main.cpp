#include <Windows.h>
#include "core/skApp.h"
#include "platform/skWindow.h"
#include "ui/skButton.h"
#include "ui/skLabel.h"
#include "ui/skCheckBox.h"
#include "ui/skSlider.h"
#include "ui/skPanel.h"
#include "ui/skProgressBar.h"
#include "ui/skTextInput.h"

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE, LPSTR, int cmdShow) {
    skApp app(hInstance, cmdShow);

    skWindow* window = app.createWindow("skWidgets Demo", 800, 600);
    if (!window) {
        MessageBoxA(nullptr, "Failed to create window.", "skWidgets", MB_ICONERROR | MB_OK);
        return 1;
    }
    HWND hwnd = window->hwnd();

    // ---- Header ----
    auto title = std::make_shared<skLabel>(30, 22, 740, 38, "skWidgets", 30.f);
    title->setColor(SkColorSetRGB(25, 25, 35));
    window->addWidget(title);

    auto subtitle = std::make_shared<skLabel>(30, 60, 740, 20, "A Skia + Win32 UI framework", 13.f);
    subtitle->setColor(SkColorSetRGB(120, 120, 135));
    window->addWidget(subtitle);

    // ---- Card panel ----
    auto panel = std::make_shared<skPanel>(24, 90, 752, 480);
    window->addWidget(panel);

    // ---- Name input ----
    auto nameLabel = std::make_shared<skLabel>(50, 112, 300, 20, "Your name", 12.f);
    nameLabel->setColor(SkColorSetRGB(80, 80, 95));
    window->addWidget(nameLabel);

    auto nameInput = std::make_shared<skTextInput>(50, 134, 340, 36, "Enter your name...");
    window->addWidget(nameInput);

    // ---- Volume slider + progress bar ----
    auto volLabel = std::make_shared<skLabel>(50, 194, 400, 20, "Volume: 65%", 12.f);
    volLabel->setColor(SkColorSetRGB(80, 80, 95));
    window->addWidget(volLabel);

    auto progBar = std::make_shared<skProgressBar>(50, 218, 340, 10);
    progBar->setValue(0.65f);
    window->addWidget(progBar);

    auto slider = std::make_shared<skSlider>(50, 242, 340, 26);
    slider->setValue(0.65f);
    slider->setOnChange([volLabel, progBar, hwnd](float v) {
        int pct = static_cast<int>(v * 100.f);
        volLabel->setText("Volume: " + std::to_string(pct) + "%");
        progBar->setValue(v);
        InvalidateRect(hwnd, nullptr, FALSE);
    });
    window->addWidget(slider);

    // ---- Checkboxes ----
    auto check1 = std::make_shared<skCheckBox>(50, 295, 300, 28, "Enable notifications");
    check1->setChecked(true);
    window->addWidget(check1);

    auto check2 = std::make_shared<skCheckBox>(50, 333, 300, 28, "Start on login");
    window->addWidget(check2);

    auto check3 = std::make_shared<skCheckBox>(50, 371, 300, 28, "Send usage statistics");
    window->addWidget(check3);

    // ---- Buttons ----
    auto saveBtn = std::make_shared<skButton>(50, 430, 110, 38, "Save");
    saveBtn->setOnClick([nameInput, hwnd]() {
        std::string msg = "Saved! Name: " + (nameInput->text().empty()
                            ? "(none)" : nameInput->text());
        MessageBoxA(hwnd, msg.c_str(), "skWidgets", MB_OK);
    });
    window->addWidget(saveBtn);

    auto cancelBtn = std::make_shared<skButton>(172, 430, 110, 38, "Cancel");
    cancelBtn->setOnClick([hwnd]() { DestroyWindow(hwnd); });
    window->addWidget(cancelBtn);

    return app.run();
}
