#include <Windows.h>
#include "core/skApp.h"
#include "platform/skWindow.h"
#include "ui/skTheme.h"
#include "ui/skButton.h"
#include "ui/skLabel.h"
#include "ui/skCheckBox.h"
#include "ui/skSlider.h"
#include "ui/skCard.h"
#include "ui/skProgressBar.h"
#include "ui/skTextInput.h"
#include "ui/skPasswordBox.h"
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
#include "ui/skImage.h"
#include "ui/skTreeView.h"
#include "ui/skSizer.h"
#include "ui/skGridSizer.h"
#include "ui/skChip.h"
#include "ui/skAvatar.h"
#include "ui/skToolBar.h"
#include "ui/skBreadcrumb.h"
#include "ui/skSideBar.h"
#include "ui/skNavigationRail.h"
#include "ui/skPopover.h"
#include "ui/skMenu.h"
#include "ui/skMenuBar.h"
#include "ui/skMessageBox.h"
#include "ui/skDrawer.h"
#include "ui/skTableView.h"
#include "ui/skPropertyGrid.h"
#include "ui/skSplitView.h"
#include "ui/skColorDialog.h"
#include "ui/skChartView.h"
#include "ui/skDatePicker.h"
#include "ui/skConsoleView.h"
#include "ui/skDataGrid.h"
#include <vector>
#include <memory>
#include <string>

static void addAll(skWindow* win, const skSizer& sizer) {
    for (auto& e : sizer.children()) win->addWidget(e.widget);
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE, LPSTR, int cmdShow) {
    skApp app(hInstance, cmdShow);
    skWindow* window = app.createWindow("skWidgets Demo", 900, 700);
    if (!window) return 1;
    HWND hwnd = window->hwnd();

    // =========================================================================
    // OVERLAYS
    // =========================================================================
    auto msgBox = std::make_shared<skMessageBox>(900, 700);
    msgBox->setVisible(false);
    window->addOverlay(msgBox);

    auto colorDlg = std::make_shared<skColorDialog>(900, 700);
    colorDlg->setVisible(false);
    window->addOverlay(colorDlg);

    auto datePicker = std::make_shared<skDatePicker>(900, 700);
    datePicker->setVisible(false);
    window->addOverlay(datePicker);

    auto drawer = std::make_shared<skDrawer>(900, 700, 240);
    drawer->setTitle("Navigation");
    drawer->addItem("Home",     [window]() { window->showToast("Home");     });
    drawer->addItem("Profile",  [window]() { window->showToast("Profile");  });
    drawer->addItem("Settings", [window]() { window->showToast("Settings"); });
    drawer->addItem("Help",     [window]() { window->showToast("Help");     });
    drawer->addItem("Sign out", [window]() { window->showToast("Signed out"); });
    window->addOverlay(drawer);

    auto fileMenu = std::make_shared<skMenu>();
    fileMenu->addItem("New",  [](){});
    fileMenu->addItem("Open", [](){});
    fileMenu->addSeparator();
    fileMenu->addItem("Exit", [hwnd]() { DestroyWindow(hwnd); });
    window->addOverlay(fileMenu);

    auto editMenu = std::make_shared<skMenu>();
    editMenu->addItem("Cut",   [](){}, false);
    editMenu->addItem("Copy",  [](){}, false);
    editMenu->addItem("Paste", [](){}, false);
    window->addOverlay(editMenu);

    auto helpMenu = std::make_shared<skMenu>();
    helpMenu->addItem("About skWidgets", [msgBox, hwnd]() {
        msgBox->show("skWidgets",
            "A Skia + Win32 UI framework.\nBatch 12  \xe2\x80\x94  C++17, MSVC x64.",
            skMessageType::Info);
        InvalidateRect(hwnd, nullptr, FALSE);
    });
    window->addOverlay(helpMenu);

    auto menuBar = std::make_shared<skMenuBar>(0, 0, 900, 28);
    menuBar->addMenu("File", fileMenu);
    menuBar->addMenu("Edit", editMenu);
    menuBar->addMenu("Help", helpMenu);
    window->addWidget(menuBar);

    // =========================================================================
    // HEADER
    // =========================================================================
    auto drawerBtn = std::make_shared<skButton>(6, 38, 32, 22, "\xe2\x89\xa1");
    drawerBtn->setTooltip("Open navigation drawer");
    drawerBtn->setOnClick([drawer, hwnd]() {
        drawer->open(); InvalidateRect(hwnd, nullptr, FALSE);
    });
    window->addWidget(drawerBtn);

    auto title    = std::make_shared<skLabel>(48, 46, 280, 36, "skWidgets", 28.f);
    auto badge    = std::make_shared<skBadge>(208, 58, "alpha");
    auto subtitle = std::make_shared<skLabel>(48, 84, 380, 18, "A Skia + Win32 UI framework", 12.f);
    auto ghLink   = std::make_shared<skLink>(450, 87, "Source on GitHub \xe2\x86\x97", 11.f);
    ghLink->setOnClick([window]() {
        window->showToast("Opening GitHub \xe2\x80\x94 imagine a browser launched!");
    });

    auto chipDesign = std::make_shared<skChip>(450, 56, "Design");
    auto chipCpp    = std::make_shared<skChip>(0,   56, "C++");
    auto chipSkia   = std::make_shared<skChip>(0,   56, "Skia");
    chipCpp->x  = chipDesign->x + chipDesign->w + 8;
    chipSkia->x = chipCpp->x   + chipCpp->w    + 8;

    auto av1 = std::make_shared<skAvatar>(724, 46, 16); av1->setInitials("JD");
    auto av2 = std::make_shared<skAvatar>(756, 46, 16); av2->setInitials("SK");
    auto av3 = std::make_shared<skAvatar>(788, 46, 16); av3->setInitials("AB");

    auto headerImg = std::make_shared<skImage>(826, 62, 42, 42);
    headerImg->setPlaceholder("logo.png");
    headerImg->loadFromFile("logo.png");

    auto darkChk = std::make_shared<skCheckBox>(660, 50, 124, 28, "Dark mode");
    darkChk->setTooltip("Switch between light and dark colour themes");
    darkChk->setOnChange([hwnd](bool dark) {
        skSetTheme(dark ? skTheme::dark() : skTheme::light());
        InvalidateRect(hwnd, nullptr, FALSE);
    });

    window->addWidget(title);    window->addWidget(badge);
    window->addWidget(subtitle); window->addWidget(ghLink);
    window->addWidget(chipDesign); window->addWidget(chipCpp); window->addWidget(chipSkia);
    window->addWidget(av1); window->addWidget(av2); window->addWidget(av3);
    window->addWidget(headerImg); window->addWidget(darkChk);

    // =========================================================================
    // CARD
    // =========================================================================
    auto card = std::make_shared<skCard>(24, 110, 852, 556);
    window->addWidget(card);

    // =========================================================================
    // LEFT COLUMN — always visible
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
    col.layout(50, 130, 320);

    cancelBtn->x = saveBtn->x + saveBtn->w + 12;
    cancelBtn->y = saveBtn->y;

    auto spinner = std::make_shared<skSpinner>(
        cancelBtn->x + cancelBtn->w + 12,
        cancelBtn->y + (cancelBtn->h - 28) / 2, 28);
    spinner->setTooltip("Saving...");

    addAll(window, col);
    window->addWidget(cancelBtn);
    window->addWidget(spinner);

    // =========================================================================
    // RIGHT COLUMN — 7 tabs
    // =========================================================================
    const int kTabX     = 430;
    const int kTabY     = 130;
    const int kTabW     = 440;
    const int kContentY = kTabY + 34 + 12;  // 176

    auto tabBar = std::make_shared<skTabBar>(kTabX, kTabY, kTabW, 34);
    tabBar->addTab("Settings");
    tabBar->addTab("Files");
    tabBar->addTab("Notes");
    tabBar->addTab("About");
    tabBar->addTab("Gallery");
    tabBar->addTab("Data");
    tabBar->addTab("Dev");
    window->addWidget(tabBar);

    // ---- Settings tab ----
    std::vector<std::shared_ptr<skWidget>> settingsW;
    std::shared_ptr<skDropdown> dropdown;
    {
        skSizer s(skDirection::Column, 10);
        auto tog1 = std::make_shared<skToggle>(0,0,kTabW,28,"Enable notifications");
        tog1->setChecked(true);
        tog1->setTooltip("Show desktop and email notifications");
        auto tog2 = std::make_shared<skToggle>(0,0,kTabW,28,"Start on login");
        auto chk3 = std::make_shared<skCheckBox>(0,0,kTabW,28,"Send usage statistics");
        s.add(tog1,28); s.add(tog2,28); s.add(chk3,28);

        auto themeLabel = std::make_shared<skLabel>(0,0,kTabW,18,"Accent colour",12.f);
        s.add(themeLabel,18);
        dropdown = std::make_shared<skDropdown>(0,0,kTabW,36);
        dropdown->addOption("Blue   (default)");
        dropdown->addOption("Purple");
        dropdown->addOption("Teal");
        dropdown->addOption("Orange");
        s.add(dropdown,36);

        auto sep2      = std::make_shared<skSeparator>(0,0,kTabW,1);
        auto passLabel = std::make_shared<skLabel>(0,0,kTabW,18,"Account password",12.f);
        auto passInput = std::make_shared<skPasswordBox>(0,0,kTabW,36,"Enter password...");
        passInput->setTooltip("Characters are masked \xe2\x80\x94 Tab to next field");
        s.add(sep2,1); s.add(passLabel,18); s.add(passInput,36);

        s.layout(kTabX, kContentY, kTabW);
        for (auto& e : s.children()) { window->addWidget(e.widget); settingsW.push_back(e.widget); }
        window->addOverlay(dropdown);
    }
    dropdown->setOnChange([hwnd](int idx, const std::string&) {
        auto th = skGetTheme();
        switch(idx){
            case 1: th.accent=SkColorSetRGB(130,80,220); th.accentHover=SkColorSetRGB(155,110,240); th.accentPress=SkColorSetRGB(100,50,190); break;
            case 2: th.accent=SkColorSetRGB(20,160,150); th.accentHover=SkColorSetRGB(40,185,175);  th.accentPress=SkColorSetRGB(10,130,120);  break;
            case 3: th.accent=SkColorSetRGB(220,120,20); th.accentHover=SkColorSetRGB(240,145,50);  th.accentPress=SkColorSetRGB(190,95,10);   break;
            default: th.accent=SkColorSetRGB(55,120,220); th.accentHover=SkColorSetRGB(80,150,255); th.accentPress=SkColorSetRGB(20,80,180);   break;
        }
        skSetTheme(th); InvalidateRect(hwnd, nullptr, FALSE);
    });

    // ---- Files tab ----
    std::vector<std::shared_ptr<skWidget>> filesW;
    {
        auto treeView = std::make_shared<skTreeView>(kTabX, kContentY, kTabW, 240);
        treeView->setTooltip("Click chevron to expand \xc2\xb7 arrow keys navigate");

        auto root = treeView->addRoot("skWidgets/");
        auto core = treeView->addChild(root, "core/");
            treeView->addChild(core,"skApp.h"); treeView->addChild(core,"skApp.cpp");
        auto plat = treeView->addChild(root,"platform/");
            treeView->addChild(plat,"skWindow.h"); treeView->addChild(plat,"skWindow.cpp");
        auto rend = treeView->addChild(root,"rendering/");
            treeView->addChild(rend,"skRenderContext.h"); treeView->addChild(rend,"skRenderContext.cpp");
        auto ui = treeView->addChild(root,"ui/");
        ui->expanded = false;
            treeView->addChild(ui,"skWidget.h");
            treeView->addChild(ui,"skButton / Label / Badge / Link / Chip");
            treeView->addChild(ui,"skTextInput / PasswordBox / TextArea");
            treeView->addChild(ui,"skDropdown / ListBox / TreeView");
            treeView->addChild(ui,"skTableView / PropertyGrid / DataGrid");
            treeView->addChild(ui,"skMenuBar / Menu / ToolBar");
            treeView->addChild(ui,"skDrawer / NavigationRail / SideBar");
            treeView->addChild(ui,"skSplitView / ColorDialog / DatePicker");
            treeView->addChild(ui,"skChartView / ConsoleView");
        treeView->addChild(root,"main.cpp");
        treeView->addChild(root,"CMakeLists.txt");

        treeView->setOnSelect([window](const std::string& label) {
            window->showToast("Selected: " + label);
        });
        window->addWidget(treeView);
        filesW.push_back(treeView);
    }

    // ---- Notes tab ----
    std::vector<std::shared_ptr<skWidget>> notesW;
    {
        auto notesLbl  = std::make_shared<skLabel>(kTabX, kContentY, kTabW, 18, "Notes", 12.f);
        auto textArea  = std::make_shared<skTextArea>(kTabX, kContentY+22, kTabW, 120, "Type your notes here...");
        auto conLbl    = std::make_shared<skLabel>(kTabX, kContentY+150, kTabW, 18, "Event log", 12.f);
        auto console   = std::make_shared<skConsoleView>(kTabX, kContentY+170, kTabW, 180);

        console->info("skWidgets started");
        console->success("Skia surface created (CPU raster)");
        console->info("WM_CREATE — window handle acquired");
        console->info("Timer started: 100ms tick interval");
        console->warn("logo.png not found — using placeholder");
        console->info("Layout pass complete: 7 tabs, 80+ widgets");

        auto clearConBtn = std::make_shared<skButton>(kTabX, kContentY+356, 80, 26, "Clear log");
        clearConBtn->setOnClick([console, hwnd](){
            console->clear(); InvalidateRect(hwnd, nullptr, FALSE);
        });

        window->addWidget(notesLbl);  notesW.push_back(notesLbl);
        window->addWidget(textArea);  notesW.push_back(textArea);
        window->addWidget(conLbl);    notesW.push_back(conLbl);
        window->addWidget(console);   notesW.push_back(console);
        window->addWidget(clearConBtn); notesW.push_back(clearConBtn);
    }

    // ---- About tab ----
    std::vector<std::shared_ptr<skWidget>> aboutW;
    {
        auto exp1 = std::make_shared<skExpander>(kTabX, kContentY, kTabW, 164, 36, "Framework");
        {
            const char* items[] = { "Renderer: Skia CPU raster","Platform: Win32 x64",
                "C++ standard: C++17 (MSVC)","Themes: Light / Dark",
                "Animation: 100 ms timer","License: Apache 2.0" };
            int cy = 8;
            for (auto* txt : items) { exp1->addChild(std::make_shared<skLabel>(12,cy,kTabW-24,16,txt,11.f)); cy+=20; }
        }
        window->addWidget(exp1); aboutW.push_back(exp1);

        auto exp2 = std::make_shared<skExpander>(kTabX, kContentY+164+10, kTabW, 264, 36, "Widget catalog (48 implemented)");
        exp2->setExpanded(false);
        {
            const char* items[] = {
                "Button, Label, Badge, Link, Chip",
                "Avatar, TextInput, PasswordBox, TextArea",
                "NumberInput, CheckBox, RadioButton, Toggle",
                "Slider, ProgressBar, Spinner, Separator",
                "Dropdown, ListBox, TreeView, TabBar",
                "TableView, PropertyGrid, DataGrid, SplitView",
                "ScrollPanel, Expander, Card, Image",
                "Modal, MessageBox, Toast, Popover",
                "MenuBar, Menu, ToolBar, SideBar",
                "Drawer, NavigationRail, ColorDialog",
                "DatePicker, ChartView, ConsoleView",
                "Breadcrumb, GridSizer, StatusBar"
            };
            int cy = 8;
            for (auto* txt : items) { exp2->addChild(std::make_shared<skLabel>(12,cy,kTabW-24,16,txt,11.f)); cy+=20; }
        }
        window->addWidget(exp2); aboutW.push_back(exp2);
    }

    // ---- Gallery tab ----
    std::vector<std::shared_ptr<skWidget>> galleryW;
    {
        // Breadcrumb
        auto bc = std::make_shared<skBreadcrumb>(kTabX, kContentY, kTabW, 24);
        bc->addCrumb("Home"); bc->addCrumb("Gallery"); bc->addCrumb("Controls");
        bc->setOnClick([window](int, const std::string& lbl) { window->showToast("Navigate: " + lbl); });
        window->addWidget(bc); galleryW.push_back(bc);

        // Row: chips
        auto chipLbl = std::make_shared<skLabel>(kTabX, kContentY+30, 40, 16, "Chips", 11.f);
        window->addWidget(chipLbl); galleryW.push_back(chipLbl);
        auto gc1 = std::make_shared<skChip>(kTabX+50, kContentY+28, "Design"); gc1->setOnClick([window](){ window->showToast("Design"); });
        auto gc2 = std::make_shared<skChip>(0,         kContentY+28, "C++");    gc2->setOnClick([window](){ window->showToast("C++"); });
        auto gc3 = std::make_shared<skChip>(0,         kContentY+28, "Skia");   gc3->setOnClick([window](){ window->showToast("Skia"); });
        auto gc4 = std::make_shared<skChip>(0,         kContentY+28, "Win32");  gc4->setOnClick([window](){ window->showToast("Win32"); });
        gc2->x = gc1->x + gc1->w + 8;
        gc3->x = gc2->x + gc2->w + 8;
        gc4->x = gc3->x + gc3->w + 8;
        for (auto& c : {gc1,gc2,gc3,gc4}) { window->addWidget(c); galleryW.push_back(c); }

        // Row: avatars + toolbar
        auto avLbl = std::make_shared<skLabel>(kTabX, kContentY+56, 50, 16, "Avatars", 11.f);
        window->addWidget(avLbl); galleryW.push_back(avLbl);
        auto gav1=std::make_shared<skAvatar>(kTabX+56, kContentY+52, 14); gav1->setInitials("JD");
        auto gav2=std::make_shared<skAvatar>(kTabX+86, kContentY+52, 14); gav2->setInitials("SK");
        auto gav3=std::make_shared<skAvatar>(kTabX+116, kContentY+52, 14); gav3->setInitials("AB");
        window->addWidget(gav1); galleryW.push_back(gav1);
        window->addWidget(gav2); galleryW.push_back(gav2);
        window->addWidget(gav3); galleryW.push_back(gav3);

        auto toolBar = std::make_shared<skToolBar>(kTabX+150, kContentY+50, kTabW-150, 26);
        toolBar->addItem("New",  [window](){ window->showToast("New");  });
        toolBar->addItem("Open", [window](){ window->showToast("Open"); });
        toolBar->addItem("Save", [window](){ window->showToast("Save"); });
        toolBar->addSeparator();
        toolBar->addItem("Cut",  [window](){ window->showToast("Cut");  });
        window->addWidget(toolBar); galleryW.push_back(toolBar);

        // Navigation Rail
        auto navLbl = std::make_shared<skLabel>(kTabX, kContentY+84, 80, 16, "Navigation Rail", 11.f);
        window->addWidget(navLbl); galleryW.push_back(navLbl);

        auto navRail = std::make_shared<skNavigationRail>(kTabX, kContentY+102, 68, 192);
        navRail->addItem("\xe2\x8f\xb0", "Home");
        navRail->addItem("\xe2\x98\x86", "Fav");
        navRail->addItem("\xe2\x9a\x99", "Config");
        auto navContent = std::make_shared<skLabel>(
            kTabX+76, kContentY+130, kTabW-76-20, 60, "Home", 13.f);
        navRail->setOnChange([navContent, hwnd](int, const std::string& lbl){
            navContent->setText(lbl);
            InvalidateRect(hwnd, nullptr, FALSE);
        });
        window->addWidget(navRail);    galleryW.push_back(navRail);
        window->addWidget(navContent); galleryW.push_back(navContent);

        // Popover
        auto popover = std::make_shared<skPopover>(220, 90);
        popover->setTitle("Popover");
        popover->addLine("Floating overlay widget");
        popover->addLine("Click outside to close");
        popover->addLine("Or press Escape");
        window->addOverlay(popover);

        // Overlay buttons row
        auto overlayLbl = std::make_shared<skLabel>(kTabX, kContentY+302, kTabW, 16, "Overlays", 11.f);
        window->addWidget(overlayLbl); galleryW.push_back(overlayLbl);

        auto showMsgBtn = std::make_shared<skButton>(kTabX, kContentY+320, 106, 32, "MessageBox");
        showMsgBtn->setOnClick([msgBox, hwnd](){
            msgBox->show("Alert", "This is an skMessageBox.\nWarning type with icon.", skMessageType::Warning);
            InvalidateRect(hwnd, nullptr, FALSE);
        });
        window->addWidget(showMsgBtn); galleryW.push_back(showMsgBtn);

        auto showPopBtn = std::make_shared<skButton>(kTabX+114, kContentY+320, 92, 32, "Popover");
        showPopBtn->setOnClick([popover, showPopBtn, hwnd](){
            popover->openAt(showPopBtn->x + showPopBtn->w/2, showPopBtn->y);
            InvalidateRect(hwnd, nullptr, FALSE);
        });
        window->addWidget(showPopBtn); galleryW.push_back(showPopBtn);

        // Color picker
        auto colorSwatch = std::make_shared<skLabel>(kTabX+316, kContentY+320, 60, 32, "", 11.f);
        auto pickColorBtn = std::make_shared<skButton>(kTabX+214, kContentY+320, 96, 32, "Pick Color");
        pickColorBtn->setOnClick([colorDlg, colorSwatch, hwnd](){
            colorDlg->show(SkColorSetRGB(55,120,220));
            colorDlg->setOnConfirm([colorSwatch, hwnd](SkColor c){
                char hex[8]; snprintf(hex,sizeof(hex),"#%02X%02X%02X",
                    SkColorGetR(c), SkColorGetG(c), SkColorGetB(c));
                colorSwatch->setText(hex);
                InvalidateRect(hwnd, nullptr, FALSE);
            });
            InvalidateRect(hwnd, nullptr, FALSE);
        });
        window->addWidget(pickColorBtn); galleryW.push_back(pickColorBtn);
        window->addWidget(colorSwatch);  galleryW.push_back(colorSwatch);

        // Date picker
        auto dateLabel  = std::make_shared<skLabel>(kTabX, kContentY+362, 90, 16, "Date Picker", 11.f);
        auto dateResult = std::make_shared<skLabel>(kTabX+210, kContentY+360, kTabW-210, 32, "No date selected", 11.f);
        auto pickDateBtn = std::make_shared<skButton>(kTabX+96, kContentY+358, 106, 32, "Pick Date");
        pickDateBtn->setOnClick([datePicker, dateResult, hwnd](){
            datePicker->show({});
            datePicker->setOnConfirm([dateResult, hwnd](skDate d){
                char buf[32]; snprintf(buf,sizeof(buf),"%04d-%02d-%02d", d.year, d.month, d.day);
                dateResult->setText(buf);
                InvalidateRect(hwnd, nullptr, FALSE);
            });
            InvalidateRect(hwnd, nullptr, FALSE);
        });
        window->addWidget(dateLabel);  galleryW.push_back(dateLabel);
        window->addWidget(pickDateBtn); galleryW.push_back(pickDateBtn);
        window->addWidget(dateResult); galleryW.push_back(dateResult);
    }

    // ---- Data tab — SplitView: TableView + PropertyGrid ----
    std::vector<std::shared_ptr<skWidget>> dataW;
    {
        auto split = std::make_shared<skSplitView>(kTabX, kContentY, kTabW, 290, 0.55f);

        auto table = std::make_shared<skTableView>(0, 0, 0, 0);
        table->addColumn("Name",   0.32f);
        table->addColumn("Role",   0.35f);
        table->addColumn("Team",   0.33f);

        const char* emp[][3] = {
            {"Alice Kim",   "Lead Engineer",   "Platform"},
            {"Bob Zhao",    "Designer",        "UI/UX"},
            {"Carol Osei",  "Product Manager", "Core"},
            {"Dan Rivera",  "Engineer",        "Rendering"},
            {"Eve Martins", "QA Engineer",     "Testing"},
            {"Frank Liu",   "DevOps",          "Infra"},
            {"Grace Patel", "Engineer",        "Platform"},
            {"Hank Torres", "Designer",        "UI/UX"},
            {"Iris Nguyen", "Intern",          "Core"},
            {"Jay Kim",     "Engineer",        "Rendering"},
        };
        for (auto& row : emp) table->addRow({row[0], row[1], row[2]});

        auto propGrid = std::make_shared<skPropertyGrid>(0, 0, 0, 0);
        propGrid->addProperty("Name",   "-", false);
        propGrid->addProperty("Role",   "-", false);
        propGrid->addProperty("Team",   "-", false);
        propGrid->addProperty("Status", "Active", true);
        propGrid->addProperty("Notes",  "",        true);

        table->setOnSelect([propGrid, hwnd](int, const std::vector<std::string>& cells){
            if (cells.size() >= 3) {
                propGrid->setProperty(0, cells[0]);
                propGrid->setProperty(1, cells[1]);
                propGrid->setProperty(2, cells[2]);
                InvalidateRect(hwnd, nullptr, FALSE);
            }
        });

        split->setLeft(table);
        split->setRight(propGrid);
        window->addWidget(split);
        dataW.push_back(split);

        // Inline-editable DataGrid below
        auto gridLbl = std::make_shared<skLabel>(kTabX, kContentY+298, kTabW, 18, "Inline-editable grid", 12.f);
        window->addWidget(gridLbl); dataW.push_back(gridLbl);

        auto dgrid = std::make_shared<skDataGrid>(kTabX, kContentY+318, kTabW, 180);
        dgrid->addColumn("Task",        0.40f, true);
        dgrid->addColumn("Owner",       0.28f, true);
        dgrid->addColumn("Status",      0.20f, true);
        dgrid->addColumn("ETA",         0.12f, true);
        dgrid->addRow({"Implement skWidget",    "Alice",  "Done",   "W1"});
        dgrid->addRow({"Skia raster surface",   "Dan",    "Done",   "W1"});
        dgrid->addRow({"Win32 message loop",    "Alice",  "Done",   "W2"});
        dgrid->addRow({"Theme system",          "Bob",    "Done",   "W2"});
        dgrid->addRow({"skChartView",           "Dan",    "Done",   "W11"});
        dgrid->addRow({"skDatePicker",          "Carol",  "Done",   "W11"});
        dgrid->addRow({"skDataGrid editing",    "Alice",  "Active", "W12"});
        dgrid->addRow({"skConsoleView",         "Grace",  "Active", "W12"});
        dgrid->addRow({"GPU backend (future)",  "Jay",    "Todo",   "---"});
        dgrid->setOnChange([window](int r, int c, const std::string& val){
            window->showToast("Row " + std::to_string(r+1) + ", col " + std::to_string(c+1) + ": " + val);
        });
        window->addWidget(dgrid); dataW.push_back(dgrid);
    }

    // ---- Dev tab — ChartView + ConsoleView ----
    std::vector<std::shared_ptr<skWidget>> devW;
    {
        // Chart
        auto chartLbl = std::make_shared<skLabel>(kTabX, kContentY, kTabW, 18, "Widget count by phase", 12.f);
        window->addWidget(chartLbl); devW.push_back(chartLbl);

        auto chart = std::make_shared<skChartView>(kTabX, kContentY+20, kTabW, 168);
        chart->setTitle("");
        chart->addBar("Phase 1", 5);
        chart->addBar("Phase 2", 9);
        chart->addBar("Phase 3", 3);
        chart->addBar("Phase 4", 7);
        chart->addBar("Phase 5", 4);
        chart->addBar("Phase 6", 3);
        chart->addBar("Phase 7", 9);
        chart->addBar("Phase 8", 1);
        window->addWidget(chart); devW.push_back(chart);

        // Console
        auto conLbl = std::make_shared<skLabel>(kTabX, kContentY+196, kTabW, 18, "Build log", 12.f);
        window->addWidget(conLbl); devW.push_back(conLbl);

        auto devCon = std::make_shared<skConsoleView>(kTabX, kContentY+216, kTabW, 200);
        devCon->appendLine("cmake -B build -S .", 0xFF888888);
        devCon->appendLine("-- Configuring done (0.9s)", 0xFF888888);
        devCon->success("Build finished: main.exe");
        devCon->info("Batch 12: 4 new widgets");
        devCon->info("skChartView    OK");
        devCon->info("skDatePicker   OK");
        devCon->info("skConsoleView  OK");
        devCon->info("skDataGrid     OK");
        devCon->appendLine("Total implemented: 45 / 59", 0xFFCCCCCC);
        window->addWidget(devCon); devW.push_back(devCon);

        // Buttons to add lines
        auto infoBtn    = std::make_shared<skButton>(kTabX,        kContentY+424, 86, 26, "Info");
        auto warnBtn    = std::make_shared<skButton>(kTabX+94,     kContentY+424, 86, 26, "Warning");
        auto errBtn     = std::make_shared<skButton>(kTabX+188,    kContentY+424, 86, 26, "Error");
        auto clearBtn   = std::make_shared<skButton>(kTabX+282,    kContentY+424, 86, 26, "Clear");
        infoBtn->setOnClick([devCon, hwnd](){ devCon->info("Info message appended"); InvalidateRect(hwnd,nullptr,FALSE); });
        warnBtn->setOnClick([devCon, hwnd](){ devCon->warn("Warning: something to check"); InvalidateRect(hwnd,nullptr,FALSE); });
        errBtn->setOnClick([devCon, hwnd](){  devCon->error("Error: operation failed"); InvalidateRect(hwnd,nullptr,FALSE); });
        clearBtn->setOnClick([devCon, hwnd](){ devCon->clear(); InvalidateRect(hwnd,nullptr,FALSE); });
        window->addWidget(infoBtn);  devW.push_back(infoBtn);
        window->addWidget(warnBtn);  devW.push_back(warnBtn);
        window->addWidget(errBtn);   devW.push_back(errBtn);
        window->addWidget(clearBtn); devW.push_back(clearBtn);
    }

    // =========================================================================
    // Tab visibility
    // =========================================================================
    for (auto& ww : filesW)    ww->setVisible(false);
    for (auto& ww : notesW)    ww->setVisible(false);
    for (auto& ww : aboutW)    ww->setVisible(false);
    for (auto& ww : galleryW)  ww->setVisible(false);
    for (auto& ww : dataW)     ww->setVisible(false);
    for (auto& ww : devW)      ww->setVisible(false);

    tabBar->setOnChange([=](int idx, const std::string&){
        for (auto& ww : settingsW) ww->setVisible(idx==0);
        for (auto& ww : filesW)   ww->setVisible(idx==1);
        for (auto& ww : notesW)   ww->setVisible(idx==2);
        for (auto& ww : aboutW)   ww->setVisible(idx==3);
        for (auto& ww : galleryW) ww->setVisible(idx==4);
        for (auto& ww : dataW)    ww->setVisible(idx==5);
        for (auto& ww : devW)     ww->setVisible(idx==6);
        InvalidateRect(hwnd, nullptr, FALSE);
    });

    // =========================================================================
    // STATUS BAR
    // =========================================================================
    auto statusBar = std::make_shared<skStatusBar>(24, 672, 852, 22);
    statusBar->setText("Ready", "skWidgets alpha  \xc2\xb7  Batch 12  \xc2\xb7  45 widgets");
    window->addWidget(statusBar);

    nameInput->setOnChange([statusBar, hwnd](const std::string& t){
        statusBar->setText(t.empty() ? "Ready" : "Name: " + t,
                           "skWidgets alpha  \xc2\xb7  Batch 12");
        InvalidateRect(hwnd, nullptr, FALSE);
    });

    // =========================================================================
    // MODAL — Save confirmation
    // =========================================================================
    auto modal = std::make_shared<skModal>(0, 0, 900, 700);
    modal->setVisible(false);
    window->addOverlay(modal);

    window->setOnResize([modal, statusBar, msgBox, colorDlg, datePicker, drawer, menuBar](int nw, int nh){
        modal->w    = nw; modal->h    = nh;
        msgBox->w   = nw; msgBox->h   = nh;
        colorDlg->w = nw; colorDlg->h = nh;
        datePicker->w = nw; datePicker->h = nh;
        drawer->w   = nw; drawer->h   = nh;
        menuBar->w  = nw;
        statusBar->x = 24;
        statusBar->y = nh - 26;
        statusBar->w = nw - 48;
    });

    modal->setOnConfirm([nameInput, prioInput, window, spinner](){
        spinner->runFor(20);
        std::string name = nameInput->text().empty() ? "anonymous" : nameInput->text();
        window->showToast("Saved: " + name + "  (priority " +
                          std::to_string(prioInput->value()) + ")");
    });

    saveBtn->setOnClick([nameInput, window, modal](){
        std::string name = nameInput->text().empty() ? "anonymous" : nameInput->text();
        modal->show("Confirm save", "Save changes for \"" + name + "\"?");
        InvalidateRect(window->hwnd(), nullptr, FALSE);
    });

    cancelBtn->setOnClick([hwnd](){ DestroyWindow(hwnd); });

    return app.run();
}
