#pragma once
#include "skWidget.h"
#include <string>
#include <vector>
#include <functional>

// Material Design 3 file-browser modal dialog.
// Add via skWindow::addOverlay(). Call show(startPath) to open.
class skFileDialog : public skWidget {
public:
    skFileDialog(int winW, int winH);

    void show(const std::string& startPath = "C:\\");
    void setOnConfirm(std::function<void(const std::string& path)> fn) { m_onConfirm = std::move(fn); }
    void setOnCancel (std::function<void()>                         fn) { m_onCancel  = std::move(fn); }

    void Paint(SkCanvas* canvas) override;
    bool handleEvent(const skEvent& ev) override;

private:
    struct Entry { std::string name; bool isDir; };

    std::string        m_currentPath;
    std::string        m_selectedPath;
    std::vector<Entry> m_entries;
    int  m_selected     = -1;
    int  m_lastClickIdx = -1;
    int  m_scroll       = 0;
    int  m_hovEntry     = -1;

    bool  m_sbDragging       = false;
    float m_sbDragStartY     = 0.f;
    int   m_sbDragStartScroll = 0;

    bool m_okHov = false, m_cancelHov = false, m_upHov = false;

    std::function<void(const std::string&)> m_onConfirm;
    std::function<void()>                   m_onCancel;

    // M3 dimensions (dp ≈ px at 96 DPI)
    static constexpr int   kDlgW     = 480;
    static constexpr int   kDlgH     = 520;
    static constexpr float kR        = 28.f;  // corner radius
    static constexpr float kHdrH     = 56.f;  // header: dialog title
    static constexpr float kPathH    = 48.f;  // breadcrumb path bar
    static constexpr float kRowH     = 48.f;  // M3 one-line list item height
    static constexpr float kPreviewH = 44.f;  // selected-file preview strip
    static constexpr float kActH     = 60.f;  // action bar
    static constexpr float kSbW      =  6.f;  // scrollbar width
    static constexpr float kListH    = (float)kDlgH - kHdrH - kPathH - kPreviewH - kActH;

    float dlgX()  const { return (float)x + ((float)w - kDlgW) * 0.5f; }
    float dlgY()  const { return (float)y + ((float)h - kDlgH) * 0.5f; }

    SkRect okRect()      const;
    SkRect cancelRect()  const;
    SkRect upRect()      const;
    SkRect listRect()    const;
    SkRect previewRect() const;

    int    entryAt(int py)   const;
    int    visibleRows()     const;
    int    maxScroll()       const;
    SkRect sbThumbRect()     const;

    void loadDirectory(const std::string& path);
    void navigateUp();
    void openEntry(int idx);

    void drawUpChevron(SkCanvas*, float cx, float cy, SkColor) const;
};
