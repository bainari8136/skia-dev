#pragma once
#include "skWidget.h"
#include <string>
#include <functional>

class skTextInput : public skWidget {
public:
    skTextInput(int x, int y, int w, int h, std::string placeholder = "");

    void Paint(SkCanvas* canvas) override;
    void OnEvent(const skEvent& event) override;

    bool canFocus()        const override { return true; }
    void onFocusGained()         override { m_focused = true; m_cursorVisible = true; }
    void onFocusLost()           override { m_focused = false; }
    void onTick()                override;

    const std::string& text() const { return m_text; }
    void setText(std::string t)     { m_text = std::move(t); m_cursor = m_text.size(); }
    void setOnChange(std::function<void(const std::string&)> cb) { m_onChange = std::move(cb); }

private:
    std::string  m_text;
    std::string  m_placeholder;
    size_t       m_cursor        = 0;
    bool         m_focused       = false;
    bool         m_cursorVisible = true;
    std::function<void(const std::string&)> m_onChange;
};
