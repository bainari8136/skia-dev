#pragma once
#include "skWidget.h"
#include <functional>
#include <string>
#include <memory>

// Numeric stepper: [−]  value  [+]
class skNumberInput : public skWidget {
public:
    skNumberInput(int x, int y, int w, int h,
                  int minVal = 0, int maxVal = 100, int value = 0);

    static std::shared_ptr<skNumberInput> make(int x, int y, int w, int h,
                                                int minVal = 0, int maxVal = 100, int value = 0);

    int  value()       const { return m_value; }
    void setValue(int v);
    void setRange(int mn, int mx) { m_min = mn; m_max = mx; setValue(m_value); }
    void setStep(int s)           { m_step = s; }
    void setOnChange(std::function<void(int)> cb) { m_onChange = std::move(cb); }

    void Paint(SkCanvas* canvas) override;
    void OnEvent(const skEvent& event) override;
    bool canFocus()      const override { return true; }
    void onFocusGained()       override { m_focused = true; }
    void onFocusLost()         override { m_focused = false; }

    std::shared_ptr<skNumberInput> withValue(int v);
    std::shared_ptr<skNumberInput> range(int mn, int mx);
    std::shared_ptr<skNumberInput> step(int s);
    std::shared_ptr<skNumberInput> onChange(std::function<void(int)> cb);
    std::shared_ptr<skNumberInput> pos(int px, int py);
    std::shared_ptr<skNumberInput> size(int pw, int ph);

private:
    std::shared_ptr<skNumberInput> shared_this() {
        return std::static_pointer_cast<skNumberInput>(shared_from_this());
    }

    static constexpr int kBtnW = 36;

    int  m_value, m_min, m_max, m_step = 1;
    bool m_focused    = false;
    bool m_minusHov   = false, m_plusHov   = false;
    bool m_minusPress = false, m_plusPress  = false;
    std::function<void(int)> m_onChange;

    SkRect minusRect() const;
    SkRect plusRect()  const;
    void   increment(int delta);
};
