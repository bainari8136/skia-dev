#pragma once
#include "skWidget.h"
#include <include/core/SkColor.h>
#include <string>

class skLabel : public skWidget {
public:
    skLabel(int x, int y, int w, int h, std::string text, float fontSize = 18.0f);

    void Paint(SkCanvas* canvas) override;

    void setText(std::string text)    { m_text     = std::move(text); }
    void setFontSize(float size)      { m_fontSize = size; }
    void setColor(SkColor color)      { m_color    = color; }

private:
    std::string m_text;
    float       m_fontSize;
    SkColor     m_color = SK_ColorBLACK;
};
