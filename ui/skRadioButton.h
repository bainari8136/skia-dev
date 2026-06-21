#pragma once
#include "skWidget.h"
#include <string>
#include <functional>
#include <vector>

class skRadioGroup;

class skRadioButton : public skWidget {
public:
    skRadioButton(int x, int y, int w, int h, std::string label, skRadioGroup* group);

    void Paint(SkCanvas* canvas) override;
    void OnEvent(const skEvent& event) override;

    bool isSelected() const { return m_selected; }
    void setSelected(bool v) { m_selected = v; }
    void setOnSelect(std::function<void()> cb) { m_onSelect = std::move(cb); }

private:
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
