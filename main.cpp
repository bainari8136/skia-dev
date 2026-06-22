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
#include "ui/skFlexSizer.h"
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
#include "ui/skCanvasView.h"
#include "ui/skCodeEditor.h"
#include "ui/skFileDialog.h"
#include "ui/skStackPanel.h"
#include "ui/skFontDialog.h"
#include "ui/skMarkdownView.h"
#include "ui/skInspector.h"
#include "ui/skDockPanel.h"
#include "ui/skTooltip.h"
#include "ui/skVideoView.h"
#include "ui/skWebView.h"
#include "ui/skPdfView.h"
#include <include/core/SkPaint.h>
#include <include/core/SkPath.h>
#include <vector>
#include <memory>
#include <string>

static void addAll(skWindow* win, const skSizer& sizer) {
    for (auto& e : sizer.children()) win->addWidget(e.widget);
}
static void addAllFlex(skWindow* win, const skFlexSizer& sizer) {
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

    auto fileDlg = std::make_shared<skFileDialog>(900, 700);
    fileDlg->setVisible(false);
    window->addOverlay(fileDlg);

    auto fontDlg = std::make_shared<skFontDialog>(900, 700);
    window->addOverlay(fontDlg);

    auto tooltip = std::make_shared<skTooltip>(900, 700);
    window->addOverlay(tooltip);

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
            "A Skia + Win32 UI framework.\nBatch 13  \xe2\x80\x94  C++17, MSVC x64.",
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
    col.add(rb1,28); col.add(rb2,28); col.add(rb3,28);

    auto prioLabel = std::make_shared<skLabel>(0,0,320,18,"Priority (1-10)", 12.f);
    col.add(prioLabel, 18);
    auto prioInput = std::make_shared<skNumberInput>(0,0,160,34,1,10,5);
    prioInput->setTooltip("Arrow keys also work when focused");
    col.add(prioInput, 34);

    auto saveBtn   = std::make_shared<skButton>(0,0,110,38,"Save");
    auto cancelBtn = std::make_shared<skButton>(0,0,110,38,"Cancel");
    saveBtn->setTooltip("Save your changes");
    cancelBtn->setTooltip("Close");
    col.add(saveBtn, 38);
    col.layout(50, 130, 320);

    cancelBtn->x = saveBtn->x + saveBtn->w + 12;
    cancelBtn->y = saveBtn->y;

    auto spinner = std::make_shared<skSpinner>(
        cancelBtn->x + cancelBtn->w + 12,
        cancelBtn->y + (cancelBtn->h - 28)/2, 28);

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
    tabBar->addTab("Media");
    window->addWidget(tabBar);

    // ---- Settings tab ----
    std::vector<std::shared_ptr<skWidget>> settingsW;
    std::shared_ptr<skDropdown> dropdown;
    {
        skSizer s(skDirection::Column, 10);
        auto tog1 = std::make_shared<skToggle>(0,0,kTabW,28,"Enable notifications");
        tog1->setChecked(true);
        auto tog2 = std::make_shared<skToggle>(0,0,kTabW,28,"Start on login");
        auto chk3 = std::make_shared<skCheckBox>(0,0,kTabW,28,"Send usage statistics");
        s.add(tog1,28); s.add(tog2,28); s.add(chk3,28);

        auto thLbl = std::make_shared<skLabel>(0,0,kTabW,18,"Accent colour",12.f);
        s.add(thLbl,18);
        dropdown = std::make_shared<skDropdown>(0,0,kTabW,36);
        dropdown->addOption("Blue   (default)");
        dropdown->addOption("Purple");
        dropdown->addOption("Teal");
        dropdown->addOption("Orange");
        s.add(dropdown,36);

        auto sep2   = std::make_shared<skSeparator>(0,0,kTabW,1);
        auto pasLbl = std::make_shared<skLabel>(0,0,kTabW,18,"Account password",12.f);
        auto pasInp = std::make_shared<skPasswordBox>(0,0,kTabW,36,"Enter password...");
        s.add(sep2,1); s.add(pasLbl,18); s.add(pasInp,36);

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

    // ---- Files tab — code editor showing skWidget.h ----
    std::vector<std::shared_ptr<skWidget>> filesW;
    {
        auto topRow = std::make_shared<skBreadcrumb>(kTabX, kContentY, kTabW, 24);
        topRow->addCrumb("skia-dev"); topRow->addCrumb("ui"); topRow->addCrumb("skWidget.h");
        window->addWidget(topRow); filesW.push_back(topRow);

        auto editor = std::make_shared<skCodeEditor>(kTabX, kContentY+30, kTabW, 370);
        editor->setLanguage("cpp");
        // Load the actual skWidget.h from the project
        editor->loadFile("C:\\Users\\BAINARI 8136\\skia-dev\\ui\\skWidget.h");
        window->addWidget(editor); filesW.push_back(editor);

        // File picker to choose which file to view
        auto openFileBtn = std::make_shared<skButton>(kTabX, kContentY+408, 110, 26, "Browse files");
        openFileBtn->setOnClick([fileDlg, editor, topRow, hwnd](){
            fileDlg->show("C:\\Users\\BAINARI 8136\\skia-dev");
            fileDlg->setOnConfirm([editor, topRow, hwnd](const std::string& path){
                editor->loadFile(path);
                // update breadcrumb last crumb
                InvalidateRect(hwnd, nullptr, FALSE);
            });
            InvalidateRect(hwnd, nullptr, FALSE);
        });
        window->addWidget(openFileBtn); filesW.push_back(openFileBtn);

        auto fileLbl = std::make_shared<skLabel>(kTabX+118, kContentY+412, kTabW-118, 18,
            "C++/CMake syntax highlighting, mouse-wheel scroll", 10.f);
        window->addWidget(fileLbl); filesW.push_back(fileLbl);
    }

    // ---- Notes tab ----
    std::vector<std::shared_ptr<skWidget>> notesW;
    {
        auto notesLbl = std::make_shared<skLabel>(kTabX, kContentY,      kTabW, 18, "Notes",      12.f);
        auto textArea = std::make_shared<skTextArea>(kTabX, kContentY+22, kTabW, 116, "Type here...");
        auto conLbl   = std::make_shared<skLabel>(kTabX, kContentY+146,  kTabW, 18, "Event log",  12.f);
        auto console  = std::make_shared<skConsoleView>(kTabX, kContentY+166, kTabW, 182);

        console->info("skWidgets started");
        console->success("Skia surface created (CPU raster)");
        console->info("WM_CREATE — window handle acquired");
        console->info("Timer started: 100 ms tick interval");
        console->warn("logo.png not found — using placeholder");
        console->info("Layout pass: 7 tabs, 90+ widgets");
        console->success("Batch 13: skFlexSizer, skCanvasView, skCodeEditor, skFileDialog");

        auto clearBtn = std::make_shared<skButton>(kTabX, kContentY+356, 80, 26, "Clear log");
        clearBtn->setOnClick([console, hwnd](){ console->clear(); InvalidateRect(hwnd,nullptr,FALSE); });

        window->addWidget(notesLbl);  notesW.push_back(notesLbl);
        window->addWidget(textArea);  notesW.push_back(textArea);
        window->addWidget(conLbl);    notesW.push_back(conLbl);
        window->addWidget(console);   notesW.push_back(console);
        window->addWidget(clearBtn);  notesW.push_back(clearBtn);
    }

    // ---- About tab ----
    std::vector<std::shared_ptr<skWidget>> aboutW;
    {
        auto exp1 = std::make_shared<skExpander>(kTabX, kContentY, kTabW, 170, 36, "Framework");
        {
            const char* items[] = { "Renderer: Skia CPU raster","Platform: Win32 x64",
                "C++ standard: C++17 (MSVC)","Themes: Light / Dark",
                "Animation: 100 ms timer","License: Apache 2.0" };
            int cy = 8;
            for (auto* txt : items) { exp1->addChild(std::make_shared<skLabel>(12,cy,kTabW-24,16,txt,11.f)); cy+=20; }
        }
        window->addWidget(exp1); aboutW.push_back(exp1);

        auto mdView = std::make_shared<skMarkdownView>(kTabX, kContentY+174, kTabW, 290);
        mdView->setContent(
            "## Widget Catalog\n"
            "### Phase 1-2: Core & Input\n"
            "- Button, Label, Badge, Link, Chip\n"
            "- Avatar, TextInput, PasswordBox, TextArea\n"
            "- NumberInput, CheckBox, RadioButton, Toggle\n"
            "- Slider, ProgressBar, Spinner, Separator\n"
            "### Phase 3-4: Layout & Navigation\n"
            "- StackPanel, ScrollPanel, GridSizer, FlexSizer\n"
            "- Dropdown, TabBar, SideBar, NavigationRail\n"
            "- ToolBar, MenuBar, Menu, StatusBar, Breadcrumb\n"
            "### Phase 5-6: Data & Dialogs\n"
            "- ListBox, TreeView, TableView, PropertyGrid, DataGrid\n"
            "- Modal, MessageBox, FileDialog, ColorDialog\n"
            "- FontDialog, DatePicker\n"
            "### Phase 7-8: Modern & Developer\n"
            "- Card, Drawer, Toast, Popover, SplitView\n"
            "- ChartView, ConsoleView, CanvasView\n"
            "- CodeEditor, Inspector, MarkdownView\n"
            "---\n"
            "**Total: 53 / 59 implemented**\n"
        );
        window->addWidget(mdView); aboutW.push_back(mdView);
    }

    // ---- Gallery tab ----
    std::vector<std::shared_ptr<skWidget>> galleryW;
    {
        auto bc = std::make_shared<skBreadcrumb>(kTabX, kContentY, kTabW, 24);
        bc->addCrumb("Home"); bc->addCrumb("Gallery"); bc->addCrumb("Controls");
        bc->setOnClick([window](int, const std::string& lbl){ window->showToast("Navigate: "+lbl); });
        window->addWidget(bc); galleryW.push_back(bc);

        // Row: chips + avatars (compact)
        auto chipLbl = std::make_shared<skLabel>(kTabX, kContentY+32, 40, 16, "Chips", 11.f);
        window->addWidget(chipLbl); galleryW.push_back(chipLbl);
        auto gc1=std::make_shared<skChip>(kTabX+50,kContentY+30,"Design"); gc1->setOnClick([window](){ window->showToast("Design"); });
        auto gc2=std::make_shared<skChip>(0,        kContentY+30,"C++");    gc2->setOnClick([window](){ window->showToast("C++"); });
        auto gc3=std::make_shared<skChip>(0,        kContentY+30,"Skia");   gc3->setOnClick([window](){ window->showToast("Skia"); });
        gc2->x=gc1->x+gc1->w+8; gc3->x=gc2->x+gc2->w+8;
        for (auto& c:{gc1,gc2,gc3}) { window->addWidget(c); galleryW.push_back(c); }

        auto avLbl=std::make_shared<skLabel>(kTabX+260,kContentY+32,50,16,"Avatars",11.f);
        window->addWidget(avLbl); galleryW.push_back(avLbl);
        auto gav1=std::make_shared<skAvatar>(kTabX+318,kContentY+28,14); gav1->setInitials("JD");
        auto gav2=std::make_shared<skAvatar>(kTabX+348,kContentY+28,14); gav2->setInitials("SK");
        auto gav3=std::make_shared<skAvatar>(kTabX+378,kContentY+28,14); gav3->setInitials("AB");
        for (auto& a:{gav1,gav2,gav3}) { window->addWidget(a); galleryW.push_back(a); }

        // ToolBar
        auto toolBar = std::make_shared<skToolBar>(kTabX, kContentY+56, kTabW, 26);
        toolBar->addItem("New",  [window](){ window->showToast("New");  });
        toolBar->addItem("Open", [window](){ window->showToast("Open"); });
        toolBar->addItem("Save", [window](){ window->showToast("Save"); });
        toolBar->addSeparator();
        toolBar->addItem("Cut",  [window](){ window->showToast("Cut");  });
        toolBar->addItem("Copy", [window](){ window->showToast("Copy"); });
        window->addWidget(toolBar); galleryW.push_back(toolBar);

        // Navigation Rail
        auto navLbl = std::make_shared<skLabel>(kTabX, kContentY+90, 80, 16, "Nav Rail", 11.f);
        window->addWidget(navLbl); galleryW.push_back(navLbl);
        auto navRail = std::make_shared<skNavigationRail>(kTabX, kContentY+108, 68, 128);
        navRail->addItem("\xe2\x8f\xb0","Home");
        navRail->addItem("\xe2\x98\x86","Fav");
        auto navContent = std::make_shared<skLabel>(kTabX+76,kContentY+128,kTabW-76-20,40,"Home",13.f);
        navRail->setOnChange([navContent,hwnd](int,const std::string& lbl){ navContent->setText(lbl); InvalidateRect(hwnd,nullptr,FALSE); });
        window->addWidget(navRail);    galleryW.push_back(navRail);
        window->addWidget(navContent); galleryW.push_back(navContent);

        // Popover
        auto popover = std::make_shared<skPopover>(220,90);
        popover->setTitle("Popover");
        popover->addLine("Floating overlay widget");
        popover->addLine("Click outside to close");
        window->addOverlay(popover);

        // Overlay buttons (row 1)
        auto overlayLbl = std::make_shared<skLabel>(kTabX, kContentY+244, kTabW, 16, "Overlays", 11.f);
        window->addWidget(overlayLbl); galleryW.push_back(overlayLbl);

        auto showMsgBtn = std::make_shared<skButton>(kTabX,     kContentY+262, 100, 30, "MessageBox");
        auto showPopBtn = std::make_shared<skButton>(kTabX+108, kContentY+262, 80,  30, "Popover");
        showMsgBtn->setOnClick([msgBox,hwnd](){
            msgBox->show("Alert","This is an skMessageBox.\nWarning type with icon.",skMessageType::Warning);
            InvalidateRect(hwnd,nullptr,FALSE);
        });
        showPopBtn->setOnClick([popover,showPopBtn,hwnd](){
            popover->openAt(showPopBtn->x+showPopBtn->w/2, showPopBtn->y);
            InvalidateRect(hwnd,nullptr,FALSE);
        });
        window->addWidget(showMsgBtn); galleryW.push_back(showMsgBtn);
        window->addWidget(showPopBtn); galleryW.push_back(showPopBtn);

        // Color + Date row
        auto colorSwatch  = std::make_shared<skLabel>(kTabX+308,kContentY+262,70,30,"",11.f);
        auto pickColorBtn = std::make_shared<skButton>(kTabX+204,kContentY+262,96,30,"Pick Color");
        pickColorBtn->setOnClick([colorDlg,colorSwatch,hwnd](){
            colorDlg->show(SkColorSetRGB(55,120,220));
            colorDlg->setOnConfirm([colorSwatch,hwnd](SkColor c){
                char hex[8]; snprintf(hex,sizeof(hex),"#%02X%02X%02X",
                    SkColorGetR(c),SkColorGetG(c),SkColorGetB(c));
                colorSwatch->setText(hex); InvalidateRect(hwnd,nullptr,FALSE);
            });
            InvalidateRect(hwnd,nullptr,FALSE);
        });
        window->addWidget(pickColorBtn); galleryW.push_back(pickColorBtn);
        window->addWidget(colorSwatch);  galleryW.push_back(colorSwatch);

        // Overlay buttons (row 2)
        auto dateResult  = std::make_shared<skLabel>(kTabX+210,kContentY+300,kTabW-210,30,"No date selected",11.f);
        auto pickDateBtn = std::make_shared<skButton>(kTabX,    kContentY+300,96,30,"Pick Date");
        auto browseBtn   = std::make_shared<skButton>(kTabX+104,kContentY+300,96,30,"Browse files");
        auto fileResult  = std::make_shared<skLabel>(kTabX,kContentY+338,kTabW,16,"",10.f);

        pickDateBtn->setOnClick([datePicker,dateResult,hwnd](){
            datePicker->show({});
            datePicker->setOnConfirm([dateResult,hwnd](skDate d){
                char buf[32]; snprintf(buf,sizeof(buf),"%04d-%02d-%02d",d.year,d.month,d.day);
                dateResult->setText(buf); InvalidateRect(hwnd,nullptr,FALSE);
            });
            InvalidateRect(hwnd,nullptr,FALSE);
        });
        browseBtn->setOnClick([fileDlg,fileResult,hwnd](){
            fileDlg->show("C:\\Users\\BAINARI 8136\\skia-dev");
            fileDlg->setOnConfirm([fileResult,hwnd](const std::string& p){
                std::string short_p = p.size()>50 ? "..."+p.substr(p.size()-48) : p;
                fileResult->setText(short_p); InvalidateRect(hwnd,nullptr,FALSE);
            });
            InvalidateRect(hwnd,nullptr,FALSE);
        });

        window->addWidget(pickDateBtn); galleryW.push_back(pickDateBtn);
        window->addWidget(browseBtn);   galleryW.push_back(browseBtn);
        window->addWidget(dateResult);  galleryW.push_back(dateResult);
        window->addWidget(fileResult);  galleryW.push_back(fileResult);

        // Font dialog row
        auto fontResult  = std::make_shared<skLabel>(kTabX+104, kContentY+356, kTabW-104, 26, "Segoe UI 14pt", 11.f);
        auto pickFontBtn = std::make_shared<skButton>(kTabX,     kContentY+356, 96, 26, "Pick Font");
        pickFontBtn->setOnClick([fontDlg, fontResult, hwnd](){
            skFontSelection init;
            fontDlg->show(init);
            fontDlg->setOnConfirm([fontResult, hwnd](skFontSelection sel){
                std::string info = sel.family + "  " + std::to_string(sel.size) + "pt";
                if (sel.bold)   info += " Bold";
                if (sel.italic) info += " Italic";
                fontResult->setText(info);
                InvalidateRect(hwnd, nullptr, FALSE);
            });
            InvalidateRect(hwnd, nullptr, FALSE);
        });
        window->addWidget(pickFontBtn); galleryW.push_back(pickFontBtn);
        window->addWidget(fontResult);  galleryW.push_back(fontResult);
    }

    // ---- Data tab ----
    std::vector<std::shared_ptr<skWidget>> dataW;
    {
        auto split = std::make_shared<skSplitView>(kTabX, kContentY, kTabW, 280, 0.55f);
        auto table = std::make_shared<skTableView>(0,0,0,0);
        table->addColumn("Name", 0.32f); table->addColumn("Role",0.35f); table->addColumn("Team",0.33f);
        const char* emp[][3] = {
            {"Alice Kim","Lead Engineer","Platform"},{"Bob Zhao","Designer","UI/UX"},
            {"Carol Osei","Product Manager","Core"},{"Dan Rivera","Engineer","Rendering"},
            {"Eve Martins","QA Engineer","Testing"},{"Frank Liu","DevOps","Infra"},
            {"Grace Patel","Engineer","Platform"},{"Hank Torres","Designer","UI/UX"},
            {"Iris Nguyen","Intern","Core"},{"Jay Kim","Engineer","Rendering"},
        };
        for (auto& r : emp) table->addRow({r[0],r[1],r[2]});

        auto propGrid = std::make_shared<skPropertyGrid>(0,0,0,0);
        propGrid->addProperty("Name",  "-",false); propGrid->addProperty("Role",  "-",false);
        propGrid->addProperty("Team",  "-",false); propGrid->addProperty("Status","Active",true);
        propGrid->addProperty("Notes", "",  true);
        table->setOnSelect([propGrid,hwnd](int,const std::vector<std::string>& c){
            if (c.size()>=3){ propGrid->setProperty(0,c[0]); propGrid->setProperty(1,c[1]); propGrid->setProperty(2,c[2]); InvalidateRect(hwnd,nullptr,FALSE); }
        });
        split->setLeft(table); split->setRight(propGrid);
        window->addWidget(split); dataW.push_back(split);

        auto gridLbl = std::make_shared<skLabel>(kTabX, kContentY+288, kTabW, 18, "Inline-editable grid", 12.f);
        window->addWidget(gridLbl); dataW.push_back(gridLbl);

        auto dgrid = std::make_shared<skDataGrid>(kTabX, kContentY+308, kTabW, 186);
        dgrid->addColumn("Task",  0.40f,true); dgrid->addColumn("Owner",0.24f,true);
        dgrid->addColumn("Status",0.22f,true); dgrid->addColumn("ETA",  0.14f,true);
        dgrid->addRow({"Implement skWidget",   "Alice","Done",  "W1"});
        dgrid->addRow({"Skia raster surface",  "Dan",  "Done",  "W1"});
        dgrid->addRow({"Theme system",         "Bob",  "Done",  "W2"});
        dgrid->addRow({"skChartView",          "Dan",  "Done",  "W12"});
        dgrid->addRow({"skCodeEditor",         "Alice","Done",  "W13"});
        dgrid->addRow({"skFileDialog",         "Carol","Done",  "W13"});
        dgrid->addRow({"skFlexSizer",          "Grace","Done",  "W13"});
        dgrid->addRow({"skCanvasView",         "Hank", "Done",  "W13"});
        dgrid->addRow({"GPU backend (future)", "Jay",  "Todo",  "---"});
        dgrid->setOnChange([window](int r,int c,const std::string& v){
            window->showToast("Grid ["+std::to_string(r+1)+","+std::to_string(c+1)+"]: "+v);
        });
        window->addWidget(dgrid); dataW.push_back(dgrid);
    }

    // ---- Dev tab — FlexSizer side-by-side chart+console, canvas draw area ----
    std::vector<std::shared_ptr<skWidget>> devW;
    {
        // Chart and console side by side via FlexSizer (Row)
        auto chart   = std::make_shared<skChartView>(0,0,0,0);
        auto devCon  = std::make_shared<skConsoleView>(0,0,0,0);

        skFlexSizer flexRow(skFlexSizer::Direction::Row, 8);
        flexRow.addFlex(chart,  0.52f);
        flexRow.addFlex(devCon, 0.48f);
        flexRow.layout(kTabX, kContentY, kTabW, 200);
        window->addWidget(chart);  devW.push_back(chart);
        window->addWidget(devCon); devW.push_back(devCon);

        chart->setTitle("Widgets per phase");
        chart->addBar("Ph1",5); chart->addBar("Ph2",9); chart->addBar("Ph3",3);
        chart->addBar("Ph4",7); chart->addBar("Ph5",5); chart->addBar("Ph6",4);
        chart->addBar("Ph7",9); chart->addBar("Ph8",2);

        devCon->appendLine("cmake --build build --config Release", 0xFF888888);
        devCon->success("Build: main.exe (Batch 14)");
        devCon->info("skStackPanel   OK");
        devCon->info("skFontDialog   OK");
        devCon->info("skMarkdownView OK");
        devCon->info("skInspector    OK");
        devCon->appendLine("Total: 53 / 59 implemented", 0xFFCCCCCC);

        // Console action buttons
        auto infoBtn  = std::make_shared<skButton>(kTabX,       kContentY+208,80,24,"Info");
        auto warnBtn  = std::make_shared<skButton>(kTabX+88,    kContentY+208,80,24,"Warn");
        auto errBtn   = std::make_shared<skButton>(kTabX+176,   kContentY+208,80,24,"Error");
        auto clrBtn   = std::make_shared<skButton>(kTabX+264,   kContentY+208,80,24,"Clear");
        infoBtn->setOnClick([devCon,hwnd](){ devCon->info("Info message appended"); InvalidateRect(hwnd,nullptr,FALSE); });
        warnBtn->setOnClick([devCon,hwnd](){ devCon->warn("Warning: check this"); InvalidateRect(hwnd,nullptr,FALSE); });
        errBtn->setOnClick([devCon,hwnd](){  devCon->error("Error: operation failed"); InvalidateRect(hwnd,nullptr,FALSE); });
        clrBtn->setOnClick([devCon,hwnd](){  devCon->clear(); InvalidateRect(hwnd,nullptr,FALSE); });
        for (auto& b:{infoBtn,warnBtn,errBtn,clrBtn}) { window->addWidget(b); devW.push_back(b); }

        // Canvas draw area (left) + Inspector (right)
        constexpr int kCvW = 258;
        auto canvasLbl = std::make_shared<skLabel>(kTabX,kContentY+240,kCvW,18,"Draw canvas (drag to paint)",12.f);
        window->addWidget(canvasLbl); devW.push_back(canvasLbl);

        auto insLbl = std::make_shared<skLabel>(kTabX+kCvW+8,kContentY+240,kTabW-kCvW-8,18,"Theme Inspector",12.f);
        window->addWidget(insLbl); devW.push_back(insLbl);

        auto inspector = std::make_shared<skInspector>(kTabX+kCvW+8, kContentY+260, kTabW-kCvW-8, 180);
        window->addWidget(inspector); devW.push_back(inspector);

        // Strokes stored in a shared vector so lambdas capture it safely
        auto strokes = std::make_shared<std::vector<std::vector<SkPoint>>>();
        auto strokeColors = std::make_shared<std::vector<SkColor>>();

        auto cv = std::make_shared<skCanvasView>(kTabX, kContentY+260, kCvW, 160);
        cv->setOnPaint([strokes, strokeColors](SkCanvas* canvas, int cw, int ch){
            // Grid background
            SkPaint gridP; gridP.setColor(SkColorSetARGB(30,100,100,255));
            gridP.setStyle(SkPaint::kStroke_Style); gridP.setStrokeWidth(0.5f);
            for (int gx = 0; gx < cw; gx += 20) canvas->drawLine((float)gx,0,(float)gx,(float)ch,gridP);
            for (int gy = 0; gy < ch; gy += 20) canvas->drawLine(0,(float)gy,(float)cw,(float)gy,gridP);
            // Draw strokes
            for (int i = 0; i < (int)strokes->size(); ++i) {
                auto& pts = (*strokes)[i];
                if (pts.empty()) continue;
                SkPaint sp; sp.setAntiAlias(true);
                sp.setColor((*strokeColors)[i]);
                sp.setStyle(SkPaint::kStroke_Style); sp.setStrokeWidth(2.5f);
                sp.setStrokeCap(SkPaint::kRound_Cap); sp.setStrokeJoin(SkPaint::kRound_Join);
                SkPath path; path.moveTo(pts[0]);
                for (size_t j=1; j<pts.size(); ++j) path.lineTo(pts[j]);
                canvas->drawPath(path, sp);
            }
        });

        // Cycle through a few colors
        auto penColor = std::make_shared<SkColor>(0xFF3778DC);
        const SkColor palette[] = {0xFF3778DC, 0xFFE05050, 0xFF44A862, 0xFFE0A030, 0xFFAA44CC};
        auto palIdx = std::make_shared<int>(0);

        cv->setOnMouseDown([strokes, strokeColors, penColor](int lx, int ly){
            strokes->push_back({ SkPoint::Make((float)lx,(float)ly) });
            strokeColors->push_back(*penColor);
        });
        cv->setOnMouseMove([strokes](int lx, int ly, bool drag){
            if (drag && !strokes->empty())
                strokes->back().push_back(SkPoint::Make((float)lx,(float)ly));
        });

        window->addWidget(cv); devW.push_back(cv);

        auto clearCvBtn = std::make_shared<skButton>(kTabX, kContentY+428, 66, 22, "Clear");
        clearCvBtn->setOnClick([strokes, strokeColors, hwnd](){
            strokes->clear(); strokeColors->clear(); InvalidateRect(hwnd,nullptr,FALSE);
        });

        // Color picker buttons for canvas
        auto colorBtns = std::make_shared<std::vector<std::shared_ptr<skWidget>>>();
        for (int ci = 0; ci < 5; ++ci) {
            SkColor pc = palette[ci];
            auto cb = std::make_shared<skButton>(kTabX+74+ci*48, kContentY+428, 40, 22, "  ");
            cb->setOnClick([penColor, pc, hwnd](){ *penColor = pc; InvalidateRect(hwnd,nullptr,FALSE); });
            window->addWidget(cb); devW.push_back(cb);
        }

        window->addWidget(clearCvBtn); devW.push_back(clearCvBtn);
    }

    // ---- Media tab — VideoView + DockPanel(WebView+PdfView) ----
    std::vector<std::shared_ptr<skWidget>> mediaW;
    {
        // VideoView (top half)
        auto vid = std::make_shared<skVideoView>(kTabX, kContentY, kTabW, 190);
        vid->loadFile("C:\\sample\\demo.mp4");
        vid->setDuration(183.f);
        window->addWidget(vid); mediaW.push_back(vid);

        // "Tooltip demo" button
        auto ttBtn = std::make_shared<skButton>(kTabX, kContentY+198, 110, 24, "Show Tooltip");
        ttBtn->setOnClick([tooltip, ttBtn, hwnd](){
            tooltip->showAt(ttBtn->x + ttBtn->w/2, ttBtn->y, "Hello from skTooltip!", 25);
            InvalidateRect(hwnd, nullptr, FALSE);
        });
        window->addWidget(ttBtn); mediaW.push_back(ttBtn);

        // DockPanel (bottom half): WebView left, PdfView right, label top
        constexpr int kDockY = kContentY + 230;
        constexpr int kDockH = 320;
        auto dock = std::make_shared<skDockPanel>(kTabX, kDockY, kTabW, kDockH);

        auto dockLbl = std::make_shared<skLabel>(0,0,0,0,"Dock panel: Web (left)  ·  PDF (right)", 11.f);
        auto webView = std::make_shared<skWebView>(0,0,0,0);
        auto pdfView = std::make_shared<skPdfView>(0,0,0,0);

        webView->navigate("https://skia.org/docs/");
        pdfView->setPageCount(12);
        pdfView->loadFile("C:\\sample\\report.pdf");

        dock->addChild(dockLbl, skDockPanel::Dock::Top,  20);
        dock->addChild(pdfView, skDockPanel::Dock::Right, kTabW/2 - 4);
        dock->addChild(webView, skDockPanel::Dock::Fill);
        dock->layout();

        window->addWidget(dock); mediaW.push_back(dock);
    }

    // =========================================================================
    // Tab visibility
    // =========================================================================
    for (auto& ww : filesW)   ww->setVisible(false);
    for (auto& ww : notesW)   ww->setVisible(false);
    for (auto& ww : aboutW)   ww->setVisible(false);
    for (auto& ww : galleryW) ww->setVisible(false);
    for (auto& ww : dataW)    ww->setVisible(false);
    for (auto& ww : devW)     ww->setVisible(false);
    for (auto& ww : mediaW)   ww->setVisible(false);

    tabBar->setOnChange([=](int idx, const std::string&){
        for (auto& ww : settingsW) ww->setVisible(idx==0);
        for (auto& ww : filesW)   ww->setVisible(idx==1);
        for (auto& ww : notesW)   ww->setVisible(idx==2);
        for (auto& ww : aboutW)   ww->setVisible(idx==3);
        for (auto& ww : galleryW) ww->setVisible(idx==4);
        for (auto& ww : dataW)    ww->setVisible(idx==5);
        for (auto& ww : devW)     ww->setVisible(idx==6);
        for (auto& ww : mediaW)   ww->setVisible(idx==7);
        InvalidateRect(hwnd, nullptr, FALSE);
    });

    // =========================================================================
    // STATUS BAR
    // =========================================================================
    auto statusBar = std::make_shared<skStatusBar>(24, 672, 852, 22);
    statusBar->setText("Ready", "skWidgets alpha  \xc2\xb7  Batch 15  \xc2\xb7  58 widgets");
    window->addWidget(statusBar);

    nameInput->setOnChange([statusBar, hwnd](const std::string& t){
        statusBar->setText(t.empty() ? "Ready" : "Name: " + t, "skWidgets alpha  \xc2\xb7  Batch 15");
        InvalidateRect(hwnd, nullptr, FALSE);
    });

    // =========================================================================
    // MODAL
    // =========================================================================
    auto modal = std::make_shared<skModal>(0, 0, 900, 700);
    modal->setVisible(false);
    window->addOverlay(modal);

    window->setOnResize([modal,statusBar,msgBox,colorDlg,datePicker,fileDlg,fontDlg,tooltip,drawer,menuBar](int nw,int nh){
        modal->w=nw;      modal->h=nh;
        msgBox->w=nw;     msgBox->h=nh;
        colorDlg->w=nw;   colorDlg->h=nh;
        datePicker->w=nw; datePicker->h=nh;
        fileDlg->w=nw;    fileDlg->h=nh;
        fontDlg->w=nw;    fontDlg->h=nh;
        tooltip->w=nw;    tooltip->h=nh;
        drawer->w=nw;     drawer->h=nh;
        menuBar->w=nw;
        statusBar->x=24; statusBar->y=nh-26; statusBar->w=nw-48;
    });

    modal->setOnConfirm([nameInput,prioInput,window,spinner](){
        spinner->runFor(20);
        std::string name = nameInput->text().empty() ? "anonymous" : nameInput->text();
        window->showToast("Saved: "+name+"  (priority "+std::to_string(prioInput->value())+")");
    });

    saveBtn->setOnClick([nameInput, window, modal](){
        std::string name = nameInput->text().empty() ? "anonymous" : nameInput->text();
        modal->show("Confirm save", "Save changes for \""+name+"\"?");
        InvalidateRect(window->hwnd(), nullptr, FALSE);
    });
    cancelBtn->setOnClick([hwnd](){ DestroyWindow(hwnd); });

    return app.run();
}
