#pragma once
#include "skWidget.h"
#include <include/core/SkColor.h>
#include <string>
#include <functional>
#include <memory>

// Auto-sized pill label. Optionally clickable.
class skChip : public skWidget {
public:
    // Width is computed from label text at construction time.
    skChip(int x, int y, const std::string& label, float fontSize = 12.f);

    static std::shared_ptr<skChip> make(int x, int y, const std::string& label,
                                         float fontSize = 12.f);

    // Override default accent-tinted colors.
    void setColors(SkColor bg, SkColor fg) { m_bg = bg; m_fg = fg; }
    void setOnClick(std::function<void()> fn) { m_onClick = fn; }

    void Paint(SkCanvas* canvas) override;
    void OnEvent(const skEvent& ev) override;
    void onMouseLeave() override { m_hover = false; }

    std::shared_ptr<skChip> colors(SkColor bg, SkColor fg);
    std::shared_ptr<skChip> onClick(std::function<void()> fn);
    std::shared_ptr<skChip> pos(int px, int py);

private:
    std::shared_ptr<skChip> shared_this() {
        return std::static_pointer_cast<skChip>(shared_from_this());
    }

    std::string           m_label;
    float                 m_fontSize;
    SkColor               m_bg    = SK_ColorTRANSPARENT;
    SkColor               m_fg    = SK_ColorTRANSPARENT;
    bool                  m_hover = false;
    std::function<void()> m_onClick;
};
