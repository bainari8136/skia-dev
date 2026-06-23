#pragma once
#include "skWidget.h"
#include <string>
#include <vector>
#include <functional>
#include <memory>

class skListBox : public skWidget {
public:
    skListBox(int x, int y, int w, int h);

    static std::shared_ptr<skListBox> make(int x, int y, int w, int h);

    void addItem(std::string item);
    void clearItems();
    int  selectedIndex()      const { return m_selected; }
    const std::string& selectedText() const;

    void setOnChange(std::function<void(int, const std::string&)> cb) { m_onChange = std::move(cb); }

    void Paint(SkCanvas* canvas) override;
    void OnEvent(const skEvent& event) override;

    bool canFocus()      const override { return true; }
    void onFocusGained()       override { m_focused = true; }
    void onFocusLost()         override { m_focused = false; m_hovered = -1; }
    void onMouseLeave()        override { m_hovered = -1; }

    std::shared_ptr<skListBox> onChange(std::function<void(int, const std::string&)> cb);
    std::shared_ptr<skListBox> pos(int px, int py);
    std::shared_ptr<skListBox> size(int pw, int ph);

private:
    std::shared_ptr<skListBox> shared_this() {
        return std::static_pointer_cast<skListBox>(shared_from_this());
    }

    static constexpr float kRowH = 30.f;
    static constexpr float kSbW  = 10.f;

    int visibleCount() const { return (int)((float)h / kRowH); }
    int maxScroll()    const;

    std::vector<std::string>                     m_items;
    int                                          m_selected = -1;
    int                                          m_hovered  = -1;
    int                                          m_scroll   = 0;
    bool                                         m_focused  = false;
    std::function<void(int, const std::string&)> m_onChange;
};
