#pragma once
#include "skWidget.h"
#include <string>
#include <functional>
#include <memory>

// Clickable inline text link — accent colour, underline on hover.
class skLink : public skWidget {
public:
    skLink(int x, int y, std::string text, float fontSize = 13.f);

    static std::shared_ptr<skLink> make(int x, int y, std::string text, float fontSize = 13.f);

    void setOnClick(std::function<void()> cb) { m_onClick = std::move(cb); }
    void Paint(SkCanvas* canvas) override;
    void OnEvent(const skEvent& event) override;

    std::shared_ptr<skLink> onClick(std::function<void()> cb);
    std::shared_ptr<skLink> pos(int px, int py);

private:
    std::shared_ptr<skLink> shared_this() {
        return std::static_pointer_cast<skLink>(shared_from_this());
    }

    std::string           m_text;
    float                 m_fontSize;
    bool                  m_hovered = false;
    std::function<void()> m_onClick;
};
