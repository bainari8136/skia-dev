#pragma once
#include "skWidget.h"
#include <string>
#include <vector>
#include <functional>

// Two-column key-value property editor.
// Click once to select; click again (or press Enter) to begin editing the value.
class skPropertyGrid : public skWidget {
public:
    skPropertyGrid(int x, int y, int w, int h);

    static std::shared_ptr<skPropertyGrid> make(int x, int y, int w, int h);

    void addProperty(const std::string& key, const std::string& value, bool editable = true);
    void setProperty(int idx, const std::string& value);
    void clearProperties();

    std::string  value(int idx)   const;
    int          selectedRow()    const { return m_selected; }

    void setOnChange(std::function<void(int, const std::string& key, const std::string& val)> fn);

    void Paint(SkCanvas* canvas) override;
    void OnEvent(const skEvent& ev) override;
    bool canFocus() const override { return true; }

    std::shared_ptr<skPropertyGrid> onChange(
        std::function<void(int, const std::string&, const std::string&)> fn);
    std::shared_ptr<skPropertyGrid> pos(int px, int py);
    std::shared_ptr<skPropertyGrid> size(int pw, int ph);

private:
    std::shared_ptr<skPropertyGrid> shared_this() {
        return std::static_pointer_cast<skPropertyGrid>(shared_from_this());
    }

    struct Prop { std::string key; std::string val; bool editable; };
    std::vector<Prop> m_props;
    int  m_selected = -1;
    int  m_scroll   = 0;
    bool m_editing  = false;

    std::function<void(int, const std::string&, const std::string&)> m_onChange;

    static constexpr float kRowH  = 26.f;
    static constexpr float kKeyFr = 0.40f;

    int visibleRows() const { return std::max(0, (int)((float)h / kRowH)); }
    int maxScroll()   const { return std::max(0, (int)m_props.size() - visibleRows()); }
    int rowAt(int py) const;
};
