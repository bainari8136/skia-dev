#pragma once
#include "skWidget.h"
#include <string>
#include <functional>
#include <vector>
#include <memory>

class skRadioGroup;

class skRadioButton : public skWidget {
public:
    skRadioButton(int x, int y, int w, int h, std::string label, skRadioGroup* group);

    static std::shared_ptr<skRadioButton> make(int x, int y, int w, int h,
                                                std::string label, skRadioGroup* group);

    void Paint(SkCanvas* canvas) override;
    void OnEvent(const skEvent& event) override;

    bool isSelected() const { return m_selected; }
    void setSelected(bool v) { m_selected = v; }
    void setOnSelect(std::function<void()> cb) { m_onSelect = std::move(cb); }

    std::shared_ptr<skRadioButton> selected(bool v);
    std::shared_ptr<skRadioButton> onSelect(std::function<void()> cb);
    std::shared_ptr<skRadioButton> pos(int px, int py);
    std::shared_ptr<skRadioButton> size(int pw, int ph);

private:
    std::shared_ptr<skRadioButton> shared_this() {
        return std::static_pointer_cast<skRadioButton>(shared_from_this());
    }

    std::string           m_label;
    skRadioGroup*         m_group;
    bool                  m_selected = false;
    bool                  m_hovered  = false;
    std::function<void()> m_onSelect;
};

// ------------------------------------------------------------------

class skRadioGroup {
public:
    void add(skRadioButton* btn);
    void select(skRadioButton* btn);
    skRadioButton* selected() const { return m_selected; }

private:
    std::vector<skRadioButton*> m_buttons;
    skRadioButton*              m_selected = nullptr;
};
