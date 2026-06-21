#pragma once
#include "skWidget.h"

class skSeparator : public skWidget {
public:
    // Draws a 1 px line.  If h > w the line is vertical, otherwise horizontal.
    skSeparator(int x, int y, int w, int h);
    void Paint(SkCanvas* canvas) override;
};
