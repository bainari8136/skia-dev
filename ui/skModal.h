#pragma once
#include "skWidget.h"
#include <string>
#include <functional>
#include <memory>
#include <include/core/SkRect.h>

// Full-window confirmation dialog overlay.
// Add via skWindow::addOverlay().  Blocks all events while visible.
class skModal : public skWidget {
public:
    // x=0, y=0, w=windowW, h=windowH
    skModal(int x, int y, int w, int h);

    static std::shared_ptr<skModal> make(int x, int y, int w, int h);

    void show(const std::string& title, const std::string& message);
    void setOnConfirm(std::function<void()> cb) { m_onConfirm = std::move(cb); }
    void setOnCancel (std::function<void()> cb) { m_onCancel  = std::move(cb); }

    void Paint(SkCanvas* canvas) override;
    bool handleEvent(const skEvent& ev) override;

    std::shared_ptr<skModal> onConfirm(std::function<void()> cb);
    std::shared_ptr<skModal> onCancel(std::function<void()> cb);
    std::shared_ptr<skModal> pos(int px, int py);
    std::shared_ptr<skModal> size(int pw, int ph);

private:
    std::shared_ptr<skModal> shared_this() {
        return std::static_pointer_cast<skModal>(shared_from_this());
    }

    std::string m_title;
    std::string m_message;
    bool        m_confirmHov = false;
    bool        m_cancelHov  = false;

    std::function<void()> m_onConfirm;
    std::function<void()> m_onCancel;

    SkRect dialogRect()  const;
    SkRect confirmRect() const;
    SkRect cancelRect()  const;
};
