#pragma once
#include "skWidget.h"
#include <memory>

class skSeparator : public skWidget {
public:
    // Draws a 1 px line.  If h > w the line is vertical, otherwise horizontal.
    skSeparator(int x, int y, int w, int h);

    static std::shared_ptr<skSeparator> make(int x, int y, int w, int h);

    void Paint(SkCanvas* canvas) override;

    std::shared_ptr<skSeparator> pos(int px, int py);
    std::shared_ptr<skSeparator> size(int pw, int ph);

private:
    std::shared_ptr<skSeparator> shared_this() {
        return std::static_pointer_cast<skSeparator>(shared_from_this());
    }
};
