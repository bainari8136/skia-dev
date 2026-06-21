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
#include "ui/skToggle.h"
#include "ui/skNumberInput.h"
#include "ui/skScrollPanel.h"
#include "ui/skModal.h"
#include "ui/skLink.h"
#include "ui/skTextArea.h"
#include "ui/skStatusBar.h"
#include "ui/skExpander.h"
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
    auto subtitle = std::make_shared<skLabel>(32, 56, 400, 18, "A Skia + Win32 UI framework", 12.f);
    auto ghLink   = std::make_shared<skLink>(440, 59, "Source on GitHub \xe2\x86\x97", 11.f);
    ghLink->setOnClick([window]() {
        window->showToast("Opening GitHub \xe2\x80\x94 imagine a browser launched!");
    });

    window->addWidget(title);
    window->addWidget(badge);
    window->addWidget(subtitle);
    window->addWidget(ghLink);

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

    // =========================================================================
    // LEFT COLUMN — Profile (always visible)
    // =========================================================================
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
        volLabel->setText("Volume: " + std::to_string((int)(v * 100)) + "%");
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

    auto prioLabel = std::make_shared<skLabel>(0,0,320,18,"Priority (1-10)", 12.f);
    col.add(prioLabel, 18);
    auto prioInput = std::make_shared<skNumberInput>(0,0,160,34, 1, 10, 5);
    prioInput->setTooltip("Arrow keys also work when focused");
    col.add(prioInput, 34);

    auto saveBtn   = std::make_shared<skButton>(0,0,110,38,"Save");
    auto cancelBtn = std::make_shared<skButton>(0,0,110,38,"Cancel");
    saveBtn->setTooltip("Save your changes");
    cancelBtn->setTooltip("Discard changes and close");
    col.add(saveBtn, 38);

    col.layout(50, 102, 320);

    cancelBtn->x = saveBtn->x + saveBtn->w + 12;
    cancelBtn->y = saveBtn->y;

    auto spinner = std::make_shared<skSpinner>(
        cancelBtn->x + cancelBtn->w + 12,
        cancelBtn->y + (cancelBtn->h - 28) / 2,
        28);
    spinner->setTooltip("Saving...");

    addAll(window, col);
    window->addWidget(cancelBtn);
    window->addWidget(spinner);

    // =========================================================================
    // RIGHT COLUMN — 4 tabs: Settings | Files | Notes | About
    // Each tab lays out its content at the same kContentY so groups don't stack.
    // =========================================================================
    const int kTabX     = 430, kTabY = 102, kTabW = 280;
    const int kContentY = kTabY + 34 + 12;

    auto tabBar = std::make_shared<skTabBar>(kTabX, kTabY, kTabW, 34);
    tabBar->addTab("Settings");
    tabBar->addTab("Files");
    tabBar->addTab("Notes");
    tabBar->addTab("About");
    window->addWidget(tabBar);

    // ---- Settings tab ----
    std::vector<std::shared_ptr<skWidget>> settingsW;
    std::shared_ptr<skDropdown> dropdown;
    {
        skSizer s(skDirection::Column, 12);
        auto tog1 = std::make_shared<skToggle>(0,0,kTabW,28,"Enable notifications");
        tog1->setChecked(true);
        tog1->setTooltip("Show desktop and email notifications");
        auto tog2 = std::make_shared<skToggle>(0,0,kTabW,28,"Start on login");
        auto chk3 = std::make_shared<skCheckBox>(0,0,kTabW,28,"Send usage statistics");
        s.add(tog1, 28); s.add(tog2, 28); s.add(chk3, 28);

        auto themeLabel = std::make_shared<skLabel>(0,0,kTabW,18,"Accent colour", 12.f);
        s.add(themeLabel, 18);
        dropdown = std::make_shared<skDropdown>(0, 0, kTabW, 36);
        dropdown->addOption("Blue   (default)");
        dropdown->addOption("Purple");
        dropdown->addOption("Teal");
        dropdown->addOption("Orange");
        s.add(dropdown, 36);
        s.layout(kTabX, kContentY, kTabW);
        for (auto& e : s.children()) { window->addWidget(e.widget); settingsW.push_back(e.widget); }
        window->addOverlay(dropdown);
    }

    dropdown->setOnChange([hwnd](int idx, const std::string&) {
        auto th = skGetTheme();
        switch (idx) {
            case 1: th.accent=SkColorSetRGB(130, 80,220); th.accentHover=SkColorSetRGB(155,110,240); th.accentPress=SkColorSetRGB(100, 50,190); break;
            case 2: th.accent=SkColorSetRGB( 20,160,150); th.accentHover=SkColorSetRGB( 40,185,175); th.accentPress=SkColorSetRGB( 10,130,120); break;
            case 3: th.accent=SkColorSetRGB(220,120, 20); th.accentHover=SkColorSetRGB(240,145, 50); th.accentPress=SkColorSetRGB(190, 95, 10); break;
            default: th.accent=SkColorSetRGB( 55,120,220); th.accentHover=SkColorSetRGB( 80,150,255); th.accentPress=SkColorSetRGB( 20, 80,180); break;
        }
        skSetTheme(th);
        InvalidateRect(hwnd, nullptr, FALSE);
    });

    // ---- Files tab ----
    std::vector<std::shared_ptr<skWidget>> filesW;
    {
        skSizer s(skDirection::Column, 12);
        auto sep       = std::make_shared<skSeparator>(0, 0, kTabW, 1);
        auto fileLabel = std::make_shared<skLabel>(0, 0, kTabW, 18, "Recent files", 12.f);
        auto listBox   = std::make_shared<skListBox>(0, 0, kTabW, 170);
        listBox->setTooltip("Scroll with mouse wheel \xc2\xb7 arrow keys navigate");
        for (auto& f : {"document.txt","report_q2.pdf","notes.md","config.json",
                        "main.cpp","CMakeLists.txt","skWidgets.md"})
            listBox->addItem(f);
        s.add(sep, 1); s.add(fileLabel, 18); s.add(listBox, 170);
        s.layout(kTabX, kContentY, kTabW);
        for (auto& e : s.children()) { window->addWidget(e.widget); filesW.push_back(e.widget); }
    }

    // ---- Notes tab ----
    std::vector<std::shared_ptr<skWidget>> notesW;
    std::shared_ptr<skTextArea> textArea;
    {
        skSizer s(skDirection::Column, 8);
        auto notesLabel = std::make_shared<skLabel>(0, 0, kTabW, 18, "Notes", 12.f);
        s.add(notesLabel, 18);
        textArea = std::make_shared<skTextArea>(0, 0, kTabW, 184, "Type your notes here...");
        s.add(textArea, 184);
        s.layout(kTabX, kContentY, kTabW);
        for (auto& e : s.children()) { window->addWidget(e.widget); notesW.push_back(e.widget); }
    }

    // ---- About tab — two animated expanders ----
    std::vector<std::shared_ptr<skWidget>> aboutW;
    {
        // Expander 1: Framework (headerH=36, contentH=128, total=164)
        auto exp1 = std::make_shared<skExpander>(kTabX, kContentY, kTabW, 164, 36, "Framework");
        {
            const char* items[] = {
                "Renderer: Skia CPU raster",
                "Platform: Win32 x64",
                "C++ standard: C++17 (MSVC)",
                "Themes: Light / Dark",
                "Animation: 100 ms timer",
                "License: Apache 2.0",
            };
            int cy = 8;
            for (auto* txt : items) {
                exp1->addChild(std::make_shared<skLabel>(12, cy, kTabW - 24, 16, txt, 11.f));
                cy += 20;
            }
        }
        window->addWidget(exp1);
        aboutW.push_back(exp1);

        // Expander 2: Widget catalog (headerH=36, contentH=168, total=204) — collapsed by default
        int exp2Y = kContentY + 164 + 10;
        auto exp2 = std::make_shared<skExpander>(kTabX, exp2Y, kTabW, 204, 36, "Widget catalog");
        exp2->setExpanded(false);
        {
            const char* items[] = {
                "Button, Label, Badge, Link",
                "TextInput, TextArea, NumberInput",
                "CheckBox, RadioButton, Toggle",
                "Slider, ProgressBar, Separator",
                "Dropdown, ListBox, TabBar",
                "ScrollPanel, Modal, Toast",
                "Spinner, Expander, StatusBar",
                "Panel, Sizer (Row / Column)",
            };
            int cy = 8;
            for (auto* txt : items) {
                exp2->addChild(std::make_shared<skLabel>(12, cy, kTabW - 24, 16, txt, 11.f));
                cy += 20;
            }
        }
        window->addWidget(exp2);
        aboutW.push_back(exp2);
    }

    // ---- Tab visibility — Settings visible by default ----
    for (auto& ww : filesW)  ww->setVisible(false);
    for (auto& ww : notesW)  ww->setVisible(false);
    for (auto& ww : aboutW)  ww->setVisible(false);

    tabBar->setOnChange([=](int idx, const std::string&) {
        for (auto& ww : settingsW) ww->setVisible(idx == 0);
        for (auto& ww : filesW)   ww->setVisible(idx == 1);
        for (auto& ww : notesW)   ww->setVisible(idx == 2);
        for (auto& ww : aboutW)   ww->setVisible(idx == 3);
        InvalidateRect(hwnd, nullptr, FALSE);
    });

    // =========================================================================
    // STATUS BAR
    // =========================================================================
    auto statusBar = std::make_shared<skStatusBar>(24, 612, 772, 22);
    statusBar->setText("Ready", "24 widgets  \xc2\xb7  skWidgets alpha");
    window->addWidget(statusBar);

    nameInput->setOnChange([statusBar, hwnd](const std::string& t) {
        statusBar->setText(t.empty() ? "Ready" : "Name: " + t,
                           "24 widgets  \xc2\xb7  skWidgets alpha");
        InvalidateRect(hwnd, nullptr, FALSE);
    });

    // =========================================================================
    // MODAL — confirmation dialog for Save
    // =========================================================================
    auto modal = std::make_shared<skModal>(0, 0, 820, 640);
    modal->setVisible(false);
    window->addOverlay(modal);

    window->setOnResize([modal, statusBar](int nw, int nh) {
        modal->w  = nw;
        modal->h  = nh;
        statusBar->x = 24;
        statusBar->y = nh - 26;
        statusBar->w = nw - 48;
    });

    modal->setOnConfirm([nameInput, prioInput, window, spinner]() {
        spinner->runFor(20);
        std::string name = nameInput->text().empty() ? "anonymous" : nameInput->text();
        window->showToast("Saved: " + name + "  (priority " +
                          std::to_string(prioInput->value()) + ")");
    });

    saveBtn->setOnClick([nameInput, window, modal]() {
        std::string name = nameInput->text().empty() ? "anonymous" : nameInput->text();
        modal->show("Confirm save", "Save changes for \"" + name + "\"?");
        InvalidateRect(window->hwnd(), nullptr, FALSE);
    });

    cancelBtn->setOnClick([hwnd]() { DestroyWindow(hwnd); });

    return app.run();
}
