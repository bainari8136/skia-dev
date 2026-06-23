#pragma once
#include "skWidget.h"
#include <include/core/SkColor.h>
#include <string>
#include <vector>

// Scrollable terminal-style log output.
// Background is always dark (dark-panel colour). Text uses per-line colors.
class skConsoleView : public skWidget {
public:
    skConsoleView(int x, int y, int w, int h);

    void appendLine(const std::string& text, SkColor color = 0xFFCCCCCC);
    void clear();

    // Convenience wrappers
    void info   (const std::string& t) { appendLine("[INFO]  " + t, 0xFF6BAED6); }
    void warn   (const std::string& t) { appendLine("[WARN]  " + t, 0xFFF0C040); }
    void error  (const std::string& t) { appendLine("[ERROR] " + t, 0xFFFF6060); }
    void success(const std::string& t) { appendLine("[OK]    " + t, 0xFF74C476); }

    void Paint(SkCanvas* canvas) override;
    void OnEvent(const skEvent& ev) override;

private:
    struct Line { std::string text; SkColor color; };
    std::vector<Line> m_lines;
    int m_scrollOff = 0;  // lines scrolled up from the bottom; 0 = at bottom

    // Scrollbar drag state
    bool  m_sbDrag      = false;
    int   m_sbDragY     = 0;
    int   m_sbDragOff   = 0;
    float m_sbDragThumb = 0.f;
    float m_sbDragTrack = 0.f;

    static constexpr float kLineH  = 16.f;
    static constexpr float kFontSz = 10.5f;
    static constexpr float kSbW    =  8.f;
    static constexpr float kPadX   =  6.f;

    int visibleLines() const { return std::max(0, (int)((float)h / kLineH)); }
    int maxScroll()    const { return std::max(0, (int)m_lines.size() - visibleLines()); }
};
