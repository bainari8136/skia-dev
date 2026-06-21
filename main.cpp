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
#include "ui/skListBox.h"
#include "ui/skSeparator.h"
#include "ui/skTabBar.h"
#include "ui/skSpinner.h"
#include "ui/skBadge.h"
#include "ui/skSizer.h"
#include <vector>
#include <memory>

static void addAll(skWindow* win, const skSizer& sizer) {
    for (auto& e : sizer.children()) win->addWidget(e.widget);
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE, LPSTR, int cmdShow) {
    skApp app(hInstance, cmdShow);
    skWindow* window = app.createWindow("skWidgets Demo", 820, 640);
    if (!window) return 1;
    HWND hwnd = window->hwnd();

    // ---- Header ----
    auto title    = std::make_shared<skLabel>(32, 18, 300, 36, "skWidgets", 28.f);
    auto badge    = std::make_shared<skBadge>(200, 30, "alpha");
    auto subtitle = std::make_shared<skLabel>(32, 56, 560, 18, "A Skia + Win32 UI framework", 12.f);
    window->addWidget(title);
    window->addWidget(badge);
    window->addWidget(subtitle);

    // Dark-mode toggle
    auto darkChk = std::make_shared<skCheckBox>(600, 20, 200, 28, "Dark mode");
    darkChk->setTooltip("Switch between light and dark colour themes");
    darkChk->setOnChange([hwnd](bool dark) {
        skSetTheme(dark ? skTheme::dark() : skTheme::light());
        InvalidateRect(hwnd, nullptr, FALSE);
    });
    window->addWidget(darkChk);

    // ---- Card ----
    auto panel = std::make_shared<skPanel>(24, 82, 772, 526);
    window->addWidget(panel);

    // =====================================================================
    // LEFT COLUMN  (Profile content — always visible)
    // =====================================================================
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

    auto radioLabel = std::make_shared<skLabel>(0,0,320,18,"Notifications", 12.f);
    col.add(radioLabel, 18);
    auto radioGroup = std::make_shared<skRadioGroup>();
    auto rb1 = std::make_shared<skRadioButton>(0,0,320,28,"All notifications", radioGroup.get());
    auto rb2 = std::make_shared<skRadioButton>(0,0,320,28,"Mentions only",     radioGroup.get());
    auto rb3 = std::make_shared<skRadioButton>(0,0,320,28,"None",              radioGroup.get());
    radioGroup->select(rb1.get());
    col.add(rb1, 28); col.add(rb2, 28); col.add(rb3, 28);

    auto saveBtn   = std::make_shared<skButton>(0,0,110,38,"Save");
    auto cancelBtn = std::make_shared<skButton>(0,0,110,38,"Cancel");
    saveBtn->setTooltip("Save your changes");
    cancelBtn->setTooltip("Discard changes and close");
    col.add(saveBtn, 38);

    col.layout(50, 102, 320);

    // Cancel and spinner are manually placed beside Save
    cancelBtn->x = saveBtn->x + saveBtn->w + 12;
    cancelBtn->y = saveBtn->y;

    auto spinner = std::make_shared<skSpinner>(
        cancelBtn->x + cancelBtn->w + 12,
        cancelBtn->y + (cancelBtn->h - 28) / 2,
        28);
    spinner->setTooltip("Processing...");

    saveBtn->setOnClick([nameInput, hwnd, spinner]() {
        spinner->start();
        InvalidateRect(hwnd, nullptr, FALSE);
        std::string msg = "Saved!\nName: " + (nameInput->text().empty() ? "(none)" : nameInput->text());
        MessageBoxA(hwnd, msg.c_str(), "skWidgets", MB_OK);
        spinner->stop();
        InvalidateRect(hwnd, nullptr, FALSE);
    });
    cancelBtn->setOnClick([hwnd]() { DestroyWindow(hwnd); });

    addAll(window, col);
    window->addWidget(cancelBtn);
    window->addWidget(spinner);

    // =====================================================================
    // RIGHT COLUMN  (tab-switched: Settings | Files)
    // =====================================================================
    skSizer right(skDirection::Column, 12);

    // Tab bar switches between Settings and Files content
    auto tabBar = std::make_shared<skTabBar>(0,0,280,34);
    tabBar->addTab("Settings");
    tabBar->addTab("Files");
    right.add(tabBar, 34);

    // -- Settings content --
    auto chk1 = std::make_shared<skCheckBox>(0,0,280,28,"Enable notifications"); chk1->setChecked(true);
    auto chk2 = std::make_shared<skCheckBox>(0,0,280,28,"Start on login");
    auto chk3 = std::make_shared<skCheckBox>(0,0,280,28,"Send usage statistics");
    chk1->setTooltip("Show desktop and email notifications");
    right.add(chk1, 28); right.add(chk2, 28); right.add(chk3, 28);

    auto themeLabel = std::make_shared<skLabel>(0,0,280,18,"Accent colour", 12.f);
    right.add(themeLabel, 18);

    auto dropdown = std::make_shared<skDropdown>(0, 0, 280, 36);
    dropdown->addOption("Blue   (default)");
    dropdown->addOption("Purple");
    dropdown->addOption("Teal");
    dropdown->addOption("Orange");
    right.add(dropdown, 36);

    // -- Files content (hidden initially) --
    auto sep       = std::make_shared<skSeparator>(0, 0, 280, 1);
    auto fileLabel = std::make_shared<skLabel>(0, 0, 280, 18, "Recent files", 12.f);
    auto listBox   = std::make_shared<skListBox>(0, 0, 280, 150);
    listBox->setTooltip("Scroll with mouse wheel · arrow keys navigate");
    listBox->addItem("document.txt");
    listBox->addItem("report_q2.pdf");
    listBox->addItem("notes.md");
    listBox->addItem("config.json");
    listBox->addItem("main.cpp");
    listBox->addItem("CMakeLists.txt");
    listBox->addItem("skWidgets.md");
    right.add(sep,       1);
    right.add(fileLabel, 18);
    right.add(listBox,   150);

    right.layout(430, 102, 280);
    addAll(window, right);           // adds widgets including dropdown
    window->addOverlay(dropdown);    // dropdown also as overlay for on-top rendering

    // Files group starts hidden
    std::vector<std::shared_ptr<skWidget>> settingsWidgets = { chk1, chk2, chk3, themeLabel, dropdown };
    std::vector<std::shared_ptr<skWidget>> filesWidgets    = { sep, fileLabel, listBox };
    for (auto& w : filesWidgets) w->setVisible(false);

    tabBar->setOnChange([=](int idx, const std::string&) {
        bool showSettings = (idx == 0);
        for (auto& w : settingsWidgets) w->setVisible(showSettings);
        for (auto& w : filesWidgets)    w->setVisible(!showSettings);
        InvalidateRect(hwnd, nullptr, FALSE);
    });

    // Accent colour switcher
    dropdown->setOnChange([hwnd](int idx, const std::string&) {
        auto th = skGetTheme();
        switch (idx) {
            case 1: th.accent=SkColorSetRGB(130, 80,220); th.accentHover=SkColorSetRGB(155,110,240); th.accentPress=SkColorSetRGB(100, 50,190); break;
            case 2: th.accent=SkColorSetRGB( 20,160,150); th.accentHover=SkColorSetRGB( 40,185,175); th.accentPress=SkColorSetRGB( 10,130,120); break;
            case 3: th.accent=SkColorSetRGB(220,120, 20); th.accentHover=SkColorSetRGB(240,145, 50); th.accentPress=SkColorSetRGB(190, 95, 10); break;
            default: th.accent=SkColorSetRGB(55,120,220); th.accentHover=SkColorSetRGB(80,150,255);  th.accentPress=SkColorSetRGB(20, 80,180);  break;
        }
        skSetTheme(th);
        InvalidateRect(hwnd, nullptr, FALSE);
    });

    return app.run();
}
