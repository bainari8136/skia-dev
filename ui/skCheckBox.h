#pragma once
#include "skWidget.h"
#include <string>
#include <functional>

class skCheckBox : public skWidget {
public:
    skCheckBox(int x, int y, int w, int h, std::string label);

    void Paint(SkCanvas* canvas) override;
    void OnEvent(const skEvent& event) override;

    bool isChecked() const { return m_checked; }
    void setChecked(bool v) { m_checked = v; }
    void setOnChange(std::function<void(bool)> cb) { m_onChange = std::move(cb); }

private:
    std::string            m_label;
    bool                   m_checked = false;
    bool                   m_hovered = false;
    std::function<void(bool)> m_onChange;
};
