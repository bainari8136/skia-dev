#pragma once
#include "skWidget.h"
#include <string>

class skStatusBar : public skWidget {
public:
    skStatusBar(int x, int y, int w, int h);

    void setText(const std::string& left, const std::string& right = "");

    void Paint(SkCanvas* canvas) override;

private:
    std::string m_left;
    std::string m_right;
};
