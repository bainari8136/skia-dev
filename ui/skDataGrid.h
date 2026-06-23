#pragma once
#include "skWidget.h"
#include <string>
#include <vector>
#include <functional>

// Inline-editable data grid.
// Click to select a cell. Click again (or press Enter) to edit.
// Tab / Enter confirm and advance. Escape cancels. Backspace deletes.
class skDataGrid : public skWidget {
public:
    skDataGrid(int x, int y, int w, int h);

    static std::shared_ptr<skDataGrid> make(int x, int y, int w, int h);

    // relWidth: fraction of available width (0 = auto-distribute equally).
    void addColumn(const std::string& header, float relWidth = 0.f, bool editable = true);
    void addRow(std::vector<std::string> cells);
    void clearRows();
    void setCell(int row, int col, const std::string& value);

    const std::vector<std::string>& rowCells(int row) const;
    int rowCount()  const { return (int)m_rows.size(); }
    int colCount()  const { return (int)m_cols.size(); }

    void setOnChange(std::function<void(int row, int col, const std::string&)> fn);

    void Paint(SkCanvas* canvas) override;
    void OnEvent(const skEvent& ev) override;
    bool canFocus() const override { return true; }

    std::shared_ptr<skDataGrid> onChange(std::function<void(int,int,const std::string&)> fn);
    std::shared_ptr<skDataGrid> pos(int px, int py);
    std::shared_ptr<skDataGrid> size(int pw, int ph);

private:
    std::shared_ptr<skDataGrid> shared_this() {
        return std::static_pointer_cast<skDataGrid>(shared_from_this());
    }

    struct Column { std::string header; float relW; bool editable; };
    struct Row    { std::vector<std::string> cells; };

    std::vector<Column> m_cols;
    std::vector<Row>    m_rows;

    int  m_selRow  = -1, m_selCol  = -1;
    int  m_editRow = -1, m_editCol = -1;
    int  m_lastClickRow = -1, m_lastClickCol = -1;
    std::string m_editBuf;
    int  m_scroll  = 0;
    bool m_focused = false;

    std::function<void(int,int,const std::string&)> m_onChange;

    static constexpr float kHeaderH = 30.f;
    static constexpr float kRowH    = 26.f;
    static constexpr float kSbW     = 10.f;

    int  visibleRows() const;
    int  maxScroll()   const;
    void cellAt(int px, int py, int& row, int& col) const;
    void computeColWidths(float listW, std::vector<float>& out) const;
    void commitEdit();
    void cancelEdit();
    void startEdit(int row, int col);
    void navigateEdit(bool forward); // Tab forward/backward
};
