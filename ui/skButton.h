#pragma once
#include "skWidget.h"
#include <string>
#include <functional>
#include <memory>

class skButton : public skWidget {
public:
    skButton(int x, int y, int w, int h, std::string label = "Button");

    static std::shared_ptr<skButton> make(int x, int y, int w, int h, std::string label = "Button");

    void Paint(SkCanvas* canvas) override;
    void OnEvent(const skEvent& event) override;
    void onTick() override;

    // Imperative setters (existing API — kept for compatibility)
    void setOnClick(std::function<void()> cb) { m_onClick = std::move(cb); }
    void setLabel(std::string lbl)            { m_label  = std::move(lbl); }

    // Fluent interface — each returns shared_ptr<skButton> for chaining
    std::shared_ptr<skButton> label(std::string lbl);
    std::shared_ptr<skButton> onClick(std::function<void()> cb);
    std::shared_ptr<skButton> pos(int px, int py);
    std::shared_ptr<skButton> size(int pw, int ph);

private:
    std::shared_ptr<skButton> shared_this() {
        return std::static_pointer_cast<skButton>(shared_from_this());
    }

    std::string           m_label;
    bool                  m_hovered   = false;
    bool                  m_pressed   = false;
    float                 m_hoverAnim = 0.f;
    std::function<void()> m_onClick;
};
