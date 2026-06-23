#pragma once
#include "skWidget.h"
#include <string>
#include <functional>
#include <memory>

class skTextInput : public skWidget {
public:
    skTextInput(int x, int y, int w, int h, std::string placeholder = "");

    static std::shared_ptr<skTextInput> make(int x, int y, int w, int h,
                                              std::string placeholder = "");

    void Paint(SkCanvas* canvas) override;
    void OnEvent(const skEvent& event) override;

    bool canFocus()        const override { return true; }
    void onFocusGained()         override { m_focused = true; m_cursorVisible = true; m_blinkCount = 0; }
    void onFocusLost()           override { m_focused = false; }
    void onTick()                override;

    const std::string& text() const { return m_text; }
    void setText(std::string t)     { m_text = std::move(t); m_cursor = m_text.size(); }
    void setOnChange(std::function<void(const std::string&)> cb) { m_onChange = std::move(cb); }
    void setMasked(bool m)         { m_masked = m; }

    std::shared_ptr<skTextInput> withText(std::string t);
    std::shared_ptr<skTextInput> onChange(std::function<void(const std::string&)> cb);
    std::shared_ptr<skTextInput> masked(bool m);
    std::shared_ptr<skTextInput> pos(int px, int py);
    std::shared_ptr<skTextInput> size(int pw, int ph);

private:
    std::shared_ptr<skTextInput> shared_this() {
        return std::static_pointer_cast<skTextInput>(shared_from_this());
    }

    std::string  m_text;
    std::string  m_placeholder;
    size_t       m_cursor        = 0;
    bool         m_focused       = false;
    bool         m_cursorVisible = true;
    int          m_blinkCount    = 0;
    bool         m_masked        = false;
    std::function<void(const std::string&)> m_onChange;
};
