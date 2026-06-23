#pragma once
#include "skWidget.h"
#include <include/core/SkColor.h>
#include <string>
#include <memory>

class skLabel : public skWidget {
public:
    skLabel(int x, int y, int w, int h, std::string text, float fontSize = 18.f);

    static std::shared_ptr<skLabel> make(int x, int y, int w, int h,
                                          std::string text, float fontSize = 18.f);

    void Paint(SkCanvas* canvas) override;

    void setText(std::string text) { m_text     = std::move(text); }
    void setFontSize(float size)   { m_fontSize = size; }
    void setColor(SkColor color)   { m_color = color; m_colorSet = true; }

    std::shared_ptr<skLabel> text(std::string t);
    std::shared_ptr<skLabel> fontSize(float s);
    std::shared_ptr<skLabel> color(SkColor c);
    std::shared_ptr<skLabel> pos(int px, int py);
    std::shared_ptr<skLabel> size(int pw, int ph);

private:
    std::shared_ptr<skLabel> shared_this() {
        return std::static_pointer_cast<skLabel>(shared_from_this());
    }

    std::string m_text;
    float       m_fontSize;
    SkColor     m_color    = SK_ColorBLACK;
    bool        m_colorSet = false;
};
