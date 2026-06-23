#pragma once
#include "skWidget.h"
#include <string>
#include <memory>

class skStatusBar : public skWidget {
public:
    skStatusBar(int x, int y, int w, int h);

    static std::shared_ptr<skStatusBar> make(int x, int y, int w, int h);

    void setText(const std::string& left, const std::string& right = "");

    void Paint(SkCanvas* canvas) override;

    std::shared_ptr<skStatusBar> text(const std::string& left, const std::string& right = "");
    std::shared_ptr<skStatusBar> pos(int px, int py);
    std::shared_ptr<skStatusBar> size(int pw, int ph);

private:
    std::shared_ptr<skStatusBar> shared_this() {
        return std::static_pointer_cast<skStatusBar>(shared_from_this());
    }

    std::string m_left;
    std::string m_right;
};
