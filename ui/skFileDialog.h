#pragma once
#include "skWidget.h"
#include <string>
#include <vector>
#include <functional>

// File/folder browser overlay that lists real Win32 directory contents.
// Add via skWindow::addOverlay(). Call show(startPath) to open.
class skFileDialog : public skWidget {
public:
    skFileDialog(int winW, int winH);

    // startPath: absolute directory to start in (e.g. "C:\\Users").
    void show(const std::string& startPath = "C:\\");
    void setOnConfirm(std::function<void(const std::string& path)> fn) { m_onConfirm = std::move(fn); }
    void setOnCancel (std::function<void()>                         fn) { m_onCancel  = std::move(fn); }

    void Paint(SkCanvas* canvas) override;
    bool handleEvent(const skEvent& ev) override;

private:
    struct Entry { std::string name; bool isDir; };

    std::string       m_currentPath;
    std::string       m_selectedPath;
    std::vector<Entry> m_entries;
    int  m_selected     = -1;
    int  m_lastClickIdx = -1;
    int  m_scroll       = 0;

    bool m_okHov = false, m_cancelHov = false, m_upHov = false;

    std::function<void(const std::string&)> m_onConfirm;
    std::function<void()>                   m_onCancel;

    static constexpr int   kDlgW  = 420;
    static constexpr int   kDlgH  = 380;
    static constexpr float kR     = 10.f;
    static constexpr float kToolH = 36.f;   // path bar + up button
    static constexpr float kRowH  = 22.f;
    static constexpr float kSbW   =  8.f;
    static constexpr float kBtnW  = 80.f;
    static constexpr float kBtnH  = 30.f;
    static constexpr float kListY = kToolH + 4.f;
    static constexpr float kListH = (float)kDlgH - kListY - kBtnH - 24.f;

    float dlgX() const { return (float)x + ((float)w - kDlgW) / 2.f; }
    float dlgY() const { return (float)y + ((float)h - kDlgH) / 2.f; }

    SkRect okRect()     const;
    SkRect cancelRect() const;
    SkRect upRect()     const;
    SkRect listRect()   const;

    int  entryAt(int py) const;
    int  visibleRows() const;
    int  maxScroll()   const;

    void loadDirectory(const std::string& path);
    void navigateUp();
    void openEntry(int idx);  // navigate into dir or select file
};
