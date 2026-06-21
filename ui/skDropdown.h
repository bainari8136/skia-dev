#pragma once
#include "skWidget.h"
#include <string>
#include <vector>
#include <functional>

// Add to the window via addOverlay() so it paints on top and
// can consume clicks that land in the open option list.
class skDropdown : public skWidget {
public:
    skDropdown(int x, int y, int w, int h);

    void Paint(SkCanvas* canvas) override;
    void OnEvent(const skEvent& event) override;
    bool handleEvent(const skEvent& ev) override;

    void addOption(std::string option);
    int  selectedIndex() const { return m_selected; }
    const std::string& selectedText() const;
    void setOnChange(std::function<void(int, const std::string&)> cb) { m_onChange = std::move(cb); }

private:
    static constexpr float kRowH = 34.f;

    std::vector<std::string>                      m_options;
    int                                           m_selected = 0;
    bool                                          m_open     = false;
    int                                           m_hovered  = -1;
    std::function<void(int, const std::string&)>  m_onChange;

    SkRect listRect() const;
};
