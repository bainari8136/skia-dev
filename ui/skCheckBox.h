#pragma once
#include "skWidget.h"
#include <string>
#include <functional>
#include <memory>

class skCheckBox : public skWidget {
public:
    skCheckBox(int x, int y, int w, int h, std::string label);

    static std::shared_ptr<skCheckBox> make(int x, int y, int w, int h, std::string label);

    void Paint(SkCanvas* canvas) override;
    void OnEvent(const skEvent& event) override;

    bool isChecked() const { return m_checked; }
    void setChecked(bool v) { m_checked = v; }
    void setOnChange(std::function<void(bool)> cb) { m_onChange = std::move(cb); }

    std::shared_ptr<skCheckBox> checked(bool v);
    std::shared_ptr<skCheckBox> onChange(std::function<void(bool)> cb);
    std::shared_ptr<skCheckBox> pos(int px, int py);
    std::shared_ptr<skCheckBox> size(int pw, int ph);

private:
    std::shared_ptr<skCheckBox> shared_this() {
        return std::static_pointer_cast<skCheckBox>(shared_from_this());
    }

    std::string            m_label;
    bool                   m_checked = false;
    bool                   m_hovered = false;
    std::function<void(bool)> m_onChange;
};
