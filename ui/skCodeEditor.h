#pragma once
#include "skWidget.h"
#include <string>
#include <vector>

// Read-only code viewer with C++/CMake syntax highlighting and line numbers.
class skCodeEditor : public skWidget {
public:
    skCodeEditor(int x, int y, int w, int h);

    void setText    (const std::string& code);
    void loadFile   (const std::string& path);
    void setLanguage(const std::string& lang) { m_lang = lang; } // "cpp" (default), "cmake"

    void Paint(SkCanvas* canvas) override;
    void OnEvent(const skEvent& ev) override;

private:
    std::vector<std::string> m_lines;
    std::string              m_lang = "cpp";
    int m_scrollY = 0;  // first visible line
    int m_scrollX = 0;  // horizontal pixel offset

    // Vertical scrollbar drag state
    bool  m_sbDrag       = false;
    int   m_sbDragY      = 0;
    int   m_sbDragScroll = 0;
    float m_sbDragThumb  = 0.f;
    float m_sbDragTrack  = 0.f;

    static constexpr float kLineH   = 16.f;
    static constexpr float kFontSz  = 10.5f;
    static constexpr float kGutterW = 36.f;
    static constexpr float kSbH     = 8.f;   // horizontal scrollbar height
    static constexpr float kSbW     = 8.f;   // vertical scrollbar width

    int visibleLines() const { return std::max(1, (int)((float)(h - (int)kSbH) / kLineH)); }
    int maxScrollY()   const { return std::max(0, (int)m_lines.size() - visibleLines()); }

    struct Token { std::string text; SkColor color; };
    void tokenizeLine(const std::string& line, std::vector<Token>& out) const;
};
