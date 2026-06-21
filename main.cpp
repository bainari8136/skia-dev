#include <Windows.h>
#include "core/skApp.h"
#include "platform/skWindow.h"
#include "ui/skTheme.h"
#include "ui/skButton.h"
#include "ui/skLabel.h"
#include "ui/skCheckBox.h"
#include "ui/skSlider.h"
#include "ui/skPanel.h"
#include "ui/skProgressBar.h"
#include "ui/skTextInput.h"
#include "ui/skRadioButton.h"
#include "ui/skDropdown.h"
#include "ui/skSizer.h"

static void addAll(skWindow* win, const skSizer& sizer) {
    for (auto& e : sizer.children()) win->addWidget(e.widget);
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE, LPSTR, int cmdShow) {
    skApp app(hInstance, cmdShow);
    skWindow* window = app.createWindow("skWidgets Demo", 820, 640);
    if (!window) return 1;
    HWND hwnd = window->hwnd();

    // ---- Header ----
    auto title    = std::make_shared<skLabel>(32, 18, 756, 36, "skWidgets", 28.f);
    auto subtitle = std::make_shared<skLabel>(32, 54, 756, 18, "A Skia + Win32 UI framework", 12.f);
    window->addWidget(title);
    window->addWidget(subtitle);

    // Dark-mode toggle in header
    auto darkChk = std::make_shared<skCheckBox>(600, 20, 200, 28, "Dark mode");
    darkChk->setOnChange([hwnd](bool dark) {
        skSetTheme(dark ? skTheme::dark() : skTheme::light());
        InvalidateRect(hwnd, nullptr, FALSE);
    });
    window->addWidget(darkChk);

    // ---- Card ----
    auto panel = std::make_shared<skPanel>(24, 82, 772, 526);
    window->addWidget(panel);

    // Column sizer for left content (x=50, width=320)
    skSizer col(skDirection::Column, 12);

    auto nameLabel = std::make_shared<skLabel>(0,0,320,18,"Your name", 12.f);
    col.add(nameLabel, 18);
    auto nameInput = std::make_shared<skTextInput>(0,0,320,36,"Enter your name...");
    col.add(nameInput, 36);

    auto volLabel = std::make_shared<skLabel>(0,0,320,18,"Volume: 65%", 12.f);
    col.add(volLabel, 18);
    auto progBar  = std::make_shared<skProgressBar>(0,0,320,10);
    progBar->setValue(0.65f);
    col.add(progBar, 10);
    auto slider = std::make_shared<skSlider>(0,0,320,26);
    slider->setValue(0.65f);
    slider->setOnChange([volLabel, progBar, hwnd](float v) {
        volLabel->setText("Volume: " + std::to_string((int)(v*100)) + "%");
        progBar->setValue(v);
        InvalidateRect(hwnd, nullptr, FALSE);
    });
    col.add(slider, 26);

    // Notifications preference (radio group)
    auto radioLabel = std::make_shared<skLabel>(0,0,320,18,"Notifications", 12.f);
    col.add(radioLabel, 18);
    auto radioGroup = std::make_shared<skRadioGroup>();
    auto rb1 = std::make_shared<skRadioButton>(0,0,320,28,"All notifications", radioGroup.get());
    auto rb2 = std::make_shared<skRadioButton>(0,0,320,28,"Mentions only",     radioGroup.get());
    auto rb3 = std::make_shared<skRadioButton>(0,0,320,28,"None",              radioGroup.get());
    radioGroup->select(rb1.get());
    col.add(rb1, 28);
    col.add(rb2, 28);
    col.add(rb3, 28);

    // Buttons
    auto saveBtn   = std::make_shared<skButton>(0,0,110,38,"Save");
    auto cancelBtn = std::make_shared<skButton>(0,0,110,38,"Cancel");
    col.add(saveBtn, 38);

    col.layout(50, 102, 320);

    cancelBtn->x = saveBtn->x + saveBtn->w + 12;
    cancelBtn->y = saveBtn->y;

    saveBtn->setOnClick([nameInput, hwnd]() {
        std::string msg = "Saved!\nName: " + (nameInput->text().empty() ? "(none)" : nameInput->text());
        MessageBoxA(hwnd, msg.c_str(), "skWidgets", MB_OK);
    });
    cancelBtn->setOnClick([hwnd]() { DestroyWindow(hwnd); });

    addAll(window, col);
    window->addWidget(cancelBtn);

    // ---- Right column: checkboxes + dropdown ----
    skSizer right(skDirection::Column, 12);
    auto rightLabel = std::make_shared<skLabel>(0,0,280,18,"Settings", 12.f);
    right.add(rightLabel, 18);

    auto chk1 = std::make_shared<skCheckBox>(0,0,280,28,"Enable notifications"); chk1->setChecked(true);
    auto chk2 = std::make_shared<skCheckBox>(0,0,280,28,"Start on login");
    auto chk3 = std::make_shared<skCheckBox>(0,0,280,28,"Send usage statistics");
    right.add(chk1, 28);
    right.add(chk2, 28);
    right.add(chk3, 28);

    auto themeLabel = std::make_shared<skLabel>(0,0,280,18,"Accent colour", 12.f);
    right.add(themeLabel, 18);

    // Dropdown for accent — added as overlay after layout
    auto dropdown = std::make_shared<skDropdown>(0, 0, 280, 36);
    dropdown->addOption("Blue   (default)");
    dropdown->addOption("Purple");
    dropdown->addOption("Teal");
    dropdown->addOption("Orange");
    right.add(dropdown, 36);

    right.layout(430, 102, 280);
    addAll(window, right);

    // Dropdown must be an overlay so it renders / hits on top
    window->addOverlay(dropdown);

    dropdown->setOnChange([hwnd](int idx, const std::string&) {
        auto th = skGetTheme();
        switch (idx) {
            case 1: th.accent=SkColorSetRGB(130,80,220); th.accentHover=SkColorSetRGB(155,110,240); th.accentPress=SkColorSetRGB(100,50,190); break;
            case 2: th.accent=SkColorSetRGB( 20,160,150); th.accentHover=SkColorSetRGB(40,185,175); th.accentPress=SkColorSetRGB(10,130,120); break;
            case 3: th.accent=SkColorSetRGB(220,120, 20); th.accentHover=SkColorSetRGB(240,145,50); th.accentPress=SkColorSetRGB(190, 95, 10); break;
            default: th = skGetTheme(); break; // blue: keep as-is on index 0
        }
        if (idx == 0) th.accent = SkColorSetRGB(55,120,220);
        skSetTheme(th);
        InvalidateRect(hwnd, nullptr, FALSE);
    });

    return app.run();
}
