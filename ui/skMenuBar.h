#pragma once
#include "skWidget.h"
#include "skMenu.h"
#include <string>
#include <vector>
#include <memory>

class skMenuBar : public skWidget {
public:
    skMenuBar(int x, int y, int w, int h);

    void addMenu(const std::string& title, std::shared_ptr<skMenu> menu);

    void Paint(SkCanvas* canvas) override;
    void OnEvent(const skEvent& ev) override;
    void onMouseLeave() override { m_hoverIdx = -1; }

private:
    struct Entry {
        std::string             title;
        std::shared_ptr<skMenu> menu;
        float titleX = 0.f, titleW = 0.f;
    };
    std::vector<Entry> m_entries;
    int m_openIdx  = -1;
    int m_hoverIdx = -1;

    void reLayout();
    int  hitTest(int px) const;
    void syncOpen();
};
