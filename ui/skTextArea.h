#pragma once
#include "skWidget.h"
#include <string>
#include <vector>
#include <functional>

class skTextArea : public skWidget {
public:
    skTextArea(int x, int y, int w, int h, std::string placeholder = "");

    std::string text() const;
    void        setText(const std::string& t);
    void        setOnChange(std::function<void(const std::string&)> cb) { m_onChange = std::move(cb); }
    void        setPlaceholder(const std::string& p) { m_placeholder = p; }

    void Paint(SkCanvas* canvas) override;
    void OnEvent(const skEvent& ev) override;
    void onTick() override;

    bool canFocus()       const override { return true; }
    void onFocusGained()        override;
    void onFocusLost()          override;

private:
    void insertChar(char ch);
    void backspace();
    void deleteChar();
    void moveLeft();
    void moveRight();
    void moveUp();
    void moveDown();
    void scrollToCursor();

    std::vector<std::string> m_lines = {""};
    int  m_row       = 0;
    int  m_col       = 0;
    int  m_scrollRow = 0;

    bool m_focused       = false;
    bool m_cursorVisible = true;
    int  m_blinkCount    = 0;

    std::string m_placeholder;
    std::function<void(const std::string&)> m_onChange;

    static constexpr float kLineH = 20.f;
    static constexpr float kPad   =  8.f;
    static constexpr float kSbW   = 10.f;

    int visibleRows() const { return std::max(1, (int)((float)(h - (int)(kPad * 2)) / kLineH)); }
};
