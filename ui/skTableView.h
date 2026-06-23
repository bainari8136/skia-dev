#pragma once
#include "skWidget.h"
#include <string>
#include <vector>
#include <functional>

// Scrollable data table with column headers, row selection, and keyboard navigation.
class skTableView : public skWidget {
public:
    skTableView(int x, int y, int w, int h);

    // relWidth: fraction of the available width (0 = auto-distribute equally).
    void addColumn(const std::string& header, float relWidth = 0.f);
    void addRow(std::vector<std::string> cells);
    void clearRows();

    int selectedRow() const { return m_selected; }
    const std::vector<std::string>& rowCells(int idx) const;

    void setOnSelect(std::function<void(int row, const std::vector<std::string>&)> fn);

    void Paint(SkCanvas* canvas) override;
    void OnEvent(const skEvent& ev) override;
    bool canFocus() const override { return true; }

private:
    struct Column { std::string header; float relW; };
    struct Row    { std::vector<std::string> cells; };

    std::vector<Column> m_cols;
    std::vector<Row>    m_rows;
    int  m_selected = -1;
    int  m_hovered  = -1;
    int  m_scroll   = 0;
    bool m_focused  = false;

    // Vertical scrollbar drag state
    bool  m_sbDrag       = false;
    int   m_sbDragY      = 0;
    int   m_sbDragScroll = 0;
    float m_sbDragThumb  = 0.f;
    float m_sbDragTrack  = 0.f;

    std::function<void(int, const std::vector<std::string>&)> m_onSelect;

    static constexpr float kHeaderH = 30.f;
    static constexpr float kRowH    = 26.f;
    static constexpr float kSbW     = 10.f;

    int  visibleRows() const;
    int  maxScroll()   const;
    int  rowAt(int py) const;
    void computeColWidths(float listW, std::vector<float>& out) const;
};
