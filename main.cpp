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
#include "ui/skSizer.h"

// Helper: add all children of a sizer to the window at once
static void addAll(skWindow* w, const skSizer& sizer) {
    for (auto& entry : sizer.children()) w->addWidget(entry.widget);
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE, LPSTR, int cmdShow) {
    skApp app(hInstance, cmdShow);
    skWindow* window = app.createWindow("skWidgets Demo", 800, 600);
    if (!window) return 1;
    HWND hwnd = window->hwnd();

    // ---- Header (laid out manually — outside the card) ----
    auto title    = std::make_shared<skLabel>(30, 18, 740, 38, "skWidgets", 28.f);
    auto subtitle = std::make_shared<skLabel>(30, 56, 740, 20, "A Skia + Win32 UI framework", 13.f);
    title->setColor(SkColorSetRGB(20, 20, 32));
    subtitle->setColor(SkColorSetRGB(120, 120, 140));
    window->addWidget(title);
    window->addWidget(subtitle);

    // ---- Card panel ----
    auto panel = std::make_shared<skPanel>(24, 84, 752, 492);
    window->addWidget(panel);

    // ---- Column sizer for card content ----
    skSizer col(skDirection::Column, 14);

    // Name section
    auto nameLabel = std::make_shared<skLabel>(0, 0, 340, 18, "Your name", 12.f);
    nameLabel->setColor(SkColorSetRGB(75, 75, 90));
    col.add(nameLabel, 18);

    auto nameInput = std::make_shared<skTextInput>(0, 0, 340, 36, "Enter your name...");
    col.add(nameInput, 36);

    // Volume section
    auto volLabel = std::make_shared<skLabel>(0, 0, 340, 18, "Volume: 65%", 12.f);
    volLabel->setColor(SkColorSetRGB(75, 75, 90));
    col.add(volLabel, 18);

    auto progBar = std::make_shared<skProgressBar>(0, 0, 340, 10);
    progBar->setValue(0.65f);
    col.add(progBar, 10);

    auto slider = std::make_shared<skSlider>(0, 0, 340, 26);
    slider->setValue(0.65f);
    slider->setOnChange([volLabel, progBar, hwnd](float v) {
        volLabel->setText("Volume: " + std::to_string(static_cast<int>(v * 100.f)) + "%");
        progBar->setValue(v);
        InvalidateRect(hwnd, nullptr, FALSE);
    });
    col.add(slider, 26);

    // Checkboxes
    auto chk1 = std::make_shared<skCheckBox>(0, 0, 340, 28, "Enable notifications");
    chk1->setChecked(true);
    col.add(chk1, 28);

    auto chk2 = std::make_shared<skCheckBox>(0, 0, 340, 28, "Start on login");
    col.add(chk2, 28);

    auto chk3 = std::make_shared<skCheckBox>(0, 0, 340, 28, "Send usage statistics");
    col.add(chk3, 28);

    // Buttons (row sizer nested inside the column)
    skSizer btnRow(skDirection::Row, 12);
    auto saveBtn   = std::make_shared<skButton>(0, 0, 110, 38, "Save");
    auto cancelBtn = std::make_shared<skButton>(0, 0, 110, 38, "Cancel");
    btnRow.add(saveBtn,   110);
    btnRow.add(cancelBtn, 110);
    btnRow.layout(0, 0);  // positions relative; col.layout will set y

    // Add the first button to the column to anchor the row's y
    col.add(saveBtn, 38);

    // Apply column layout inside the card (padding 44px top, 50px left)
    col.layout(50, 106, 340);

    // Now fix the cancel button position to sit beside save
    cancelBtn->x = saveBtn->x + saveBtn->w + 12;
    cancelBtn->y = saveBtn->y;

    saveBtn->setOnClick([nameInput, hwnd]() {
        std::string msg = "Saved!\nName: " + (nameInput->text().empty()
                          ? "(none)" : nameInput->text());
        MessageBoxA(hwnd, msg.c_str(), "skWidgets", MB_OK);
    });
    cancelBtn->setOnClick([hwnd]() { DestroyWindow(hwnd); });

    addAll(window, col);
    window->addWidget(cancelBtn);

    return app.run();
}
