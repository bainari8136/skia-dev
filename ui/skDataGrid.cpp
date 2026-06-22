#include <Windows.h>
#include "skDataGrid.h"
#include "skTheme.h"
#include "skTypeface.h"
#include <include/core/SkPaint.h>
#include <include/core/SkFont.h>
#include <include/core/SkRRect.h>
#include <algorithm>
#include <cstring>

static sk_sp<SkTypeface>& dgTf() {
    static sk_sp<SkTypeface> tf = skGetSystemTypeface();
    return tf;
}

skDataGrid::skDataGrid(int sx, int sy, int sw, int sh)
    : skWidget(sx, sy, sw, sh) {}

void skDataGrid::addColumn(const std::string& header, float relWidth, bool editable) {
    m_cols.push_back({header, relWidth, editable});
}

void skDataGrid::addRow(std::vector<std::string> cells) {
    cells.resize(m_cols.size());
    m_rows.push_back({std::move(cells)});
}

void skDataGrid::clearRows() {
    m_rows.clear(); m_selRow = m_selCol = m_editRow = m_editCol = -1;
    m_scroll = 0;
}

void skDataGrid::setCell(int row, int col, const std::string& value) {
    if (row >= 0 && row < (int)m_rows.size() &&
        col >= 0 && col < (int)m_cols.size())
        m_rows[row].cells[col] = value;
}

const std::vector<std::string>& skDataGrid::rowCells(int row) const {
    static std::vector<std::string> empty;
    if (row >= 0 && row < (int)m_rows.size()) return m_rows[row].cells;
    return empty;
}

void skDataGrid::setOnChange(std::function<void(int,int,const std::string&)> fn) {
    m_onChange = std::move(fn);
}

int skDataGrid::visibleRows() const {
    return std::max(0, (int)((float)(h - (int)kHeaderH) / kRowH));
}
int skDataGrid::maxScroll() const {
    return std::max(0, (int)m_rows.size() - visibleRows());
}

void skDataGrid::computeColWidths(float listW, std::vector<float>& out) const {
    out.resize(m_cols.size(), 0.f);
    float fixed = 0.f; int autoCount = 0;
    for (int i = 0; i < (int)m_cols.size(); ++i) {
        if (m_cols[i].relW > 0.f) { out[i] = m_cols[i].relW * listW; fixed += out[i]; }
        else ++autoCount;
    }
    float autoW = autoCount > 0 ? (listW - fixed) / (float)autoCount : 0.f;
    for (int i = 0; i < (int)m_cols.size(); ++i)
        if (m_cols[i].relW <= 0.f) out[i] = autoW;
}

void skDataGrid::cellAt(int px, int py, int& row, int& col) const {
    row = col = -1;
    float listW = (float)w - kSbW;
    if (py < y + (int)kHeaderH) return;
    int r = (int)((float)(py - y - (int)kHeaderH) / kRowH) + m_scroll;
    if (r < 0 || r >= (int)m_rows.size()) return;
    std::vector<float> cw; computeColWidths(listW, cw);
    float cx = (float)x;
    for (int c = 0; c < (int)m_cols.size(); ++c) {
        if (px >= (int)cx && px < (int)(cx + cw[c])) { row = r; col = c; return; }
        cx += cw[c];
    }
}

void skDataGrid::commitEdit() {
    if (m_editRow < 0 || m_editCol < 0) return;
    m_rows[m_editRow].cells[m_editCol] = m_editBuf;
    if (m_onChange) m_onChange(m_editRow, m_editCol, m_editBuf);
    m_editRow = m_editCol = -1;
    m_editBuf.clear();
}

void skDataGrid::cancelEdit() {
    m_editRow = m_editCol = -1;
    m_editBuf.clear();
}

void skDataGrid::startEdit(int row, int col) {
    if (row < 0 || col < 0) return;
    if (!m_cols[col].editable) return;
    commitEdit();
    m_editRow = row; m_editCol = col;
    m_editBuf = m_rows[row].cells[col];
}

void skDataGrid::navigateEdit(bool forward) {
    commitEdit();
    if (m_selCol < 0 || m_selRow < 0) return;
    int nextCol = m_selCol + (forward ? 1 : -1);
    if (nextCol >= (int)m_cols.size()) { nextCol = 0; ++m_selRow; }
    if (nextCol < 0) { nextCol = (int)m_cols.size()-1; --m_selRow; }
    m_selRow = std::max(0, std::min((int)m_rows.size()-1, m_selRow));
    m_selCol = nextCol;
    startEdit(m_selRow, m_selCol);
}

void skDataGrid::Paint(SkCanvas* canvas) {
    const auto& th = skGetTheme();
    SkFont font(dgTf(), 12.f);  font.setEdging(SkFont::Edging::kAntiAlias);
    SkFont hfont(dgTf(), 11.f); hfont.setEdging(SkFont::Edging::kAntiAlias);

    float listW = (float)w - kSbW;
    std::vector<float> cw; computeColWidths(listW, cw);

    canvas->save();
    SkRRect rr; rr.setRectXY(SkRect::MakeXYWH((float)x,(float)y,(float)w,(float)h), 4.f, 4.f);
    canvas->clipRRect(rr, SkClipOp::kIntersect, true);

    // Header
    SkPaint hdrBg; hdrBg.setColor(th.panelBorder);
    canvas->drawRect(SkRect::MakeXYWH((float)x,(float)y,listW,kHeaderH), hdrBg);
    float cx = (float)x;
    for (int c = 0; c < (int)m_cols.size(); ++c) {
        // column separator
        if (c > 0) {
            SkPaint sp; sp.setColor(SkColorSetARGB(60,0,0,0));
            canvas->drawRect(SkRect::MakeXYWH(cx,float(y),1.f,kHeaderH), sp);
        }
        SkRect lb; hfont.measureText(m_cols[c].header.c_str(), m_cols[c].header.size(),
                                      SkTextEncoding::kUTF8, &lb);
        SkPaint hp; hp.setAntiAlias(true); hp.setColor(th.textPrimary);
        canvas->drawString(m_cols[c].header.c_str(),
            cx + 6.f - lb.left(),
            (float)y + kHeaderH/2.f - lb.height()/2.f - lb.top(),
            hfont, hp);
        cx += cw[c];
    }

    // Rows
    int vis = visibleRows();
    int end = std::min((int)m_rows.size(), m_scroll + vis);
    for (int r = m_scroll; r < end; ++r) {
        float ry = (float)y + kHeaderH + (float)(r - m_scroll) * kRowH;
        bool sel = (r == m_selRow);

        if (sel) {
            SkPaint selP; selP.setColor(SkColorSetARGB(40, SkColorGetR(th.accent),
                SkColorGetG(th.accent), SkColorGetB(th.accent)));
            canvas->drawRect(SkRect::MakeXYWH((float)x, ry, listW, kRowH), selP);
        } else if (r % 2 == 0) {
            SkPaint altP; altP.setColor(SkColorSetARGB(20,128,128,128));
            canvas->drawRect(SkRect::MakeXYWH((float)x, ry, listW, kRowH), altP);
        }

        // Row separator
        SkPaint sep; sep.setColor(th.panelBorder);
        canvas->drawRect(SkRect::MakeXYWH((float)x, ry+kRowH-1.f, listW, 1.f), sep);

        cx = (float)x;
        for (int c = 0; c < (int)m_cols.size(); ++c) {
            bool editing = (r == m_editRow && c == m_editCol);
            const std::string& cellText = editing ? m_editBuf : m_rows[r].cells[c];

            if (editing) {
                SkPaint ep; ep.setColor(th.inputBg);
                canvas->drawRect(SkRect::MakeXYWH(cx+1.f, ry+1.f, cw[c]-2.f, kRowH-2.f), ep);
                SkPaint ebrd; ebrd.setAntiAlias(true);
                ebrd.setStyle(SkPaint::kStroke_Style); ebrd.setStrokeWidth(1.5f);
                ebrd.setColor(th.accent);
                canvas->drawRect(SkRect::MakeXYWH(cx+1.f, ry+1.f, cw[c]-2.f, kRowH-2.f), ebrd);
            }

            // Clip cell content
            canvas->save();
            canvas->clipRect(SkRect::MakeXYWH(cx+4.f, ry, cw[c]-8.f, kRowH));
            SkRect tb; font.measureText(cellText.c_str(), cellText.size(),
                                         SkTextEncoding::kUTF8, &tb);
            SkPaint tp; tp.setAntiAlias(true);
            tp.setColor(editing ? th.textPrimary : (sel ? th.textPrimary : th.textPrimary));
            canvas->drawString(cellText.c_str(),
                cx + 6.f - tb.left(),
                ry + kRowH/2.f - tb.height()/2.f - tb.top(),
                font, tp);

            // Blinking cursor in editing cell
            if (editing) {
                float curX = cx + 6.f + tb.width();
                SkPaint cur; cur.setAntiAlias(true); cur.setColor(th.accent);
                canvas->drawRect(SkRect::MakeXYWH(curX+1.f, ry+4.f, 1.5f, kRowH-8.f), cur);
            }
            canvas->restore();

            // Column separator
            if (c < (int)m_cols.size()-1) {
                SkPaint csp; csp.setColor(th.panelBorder);
                canvas->drawRect(SkRect::MakeXYWH(cx+cw[c]-0.5f, ry, 1.f, kRowH), csp);
            }
            cx += cw[c];
        }
    }

    canvas->restore();

    // Outer border
    SkPaint brd; brd.setAntiAlias(true);
    brd.setStyle(SkPaint::kStroke_Style);
    brd.setStrokeWidth(m_focused ? 1.5f : 1.f);
    brd.setColor(m_focused ? th.accent : th.inputBorder);
    canvas->drawRRect(rr, brd);

    // Scrollbar
    int ms = maxScroll();
    if (ms > 0) {
        float sbX   = (float)x + (float)w - kSbW;
        float sbH   = (float)h - kHeaderH;
        float sbY   = (float)y + kHeaderH;
        SkPaint trk; trk.setColor(SkColorSetARGB(30,128,128,128));
        canvas->drawRect(SkRect::MakeXYWH(sbX, sbY, kSbW, sbH), trk);
        float ratio  = (float)vis / (float)(vis + ms);
        float thumbH = std::max(20.f, sbH * ratio);
        float thumbY = sbY + (float)m_scroll / (float)ms * (sbH - thumbH);
        SkRRect tRR; tRR.setRectXY(SkRect::MakeXYWH(sbX+1.f,thumbY+2.f,kSbW-2.f,thumbH-4.f),3.f,3.f);
        SkPaint tP; tP.setAntiAlias(true); tP.setColor(th.inputBorder);
        canvas->drawRRect(tRR, tP);
    }
}

void skDataGrid::OnEvent(const skEvent& ev) {
    switch (ev.type) {
    case skEventType::MouseDown:
        m_focused = contains(ev.x, ev.y);
        if (!m_focused) { commitEdit(); break; }
        {
            int r, c; cellAt(ev.x, ev.y, r, c);
            if (r >= 0 && c >= 0) {
                bool sameCell = (r == m_lastClickRow && c == m_lastClickCol);
                m_selRow = r; m_selCol = c;
                if (sameCell && m_editRow < 0) startEdit(r, c);
                m_lastClickRow = r; m_lastClickCol = c;
            } else {
                commitEdit();
            }
        }
        break;

    case skEventType::MouseWheel:
        if (!contains(ev.x, ev.y)) break;
        if (ev.button > 0) m_scroll = std::max(0, m_scroll - 1);
        else               m_scroll = std::min(maxScroll(), m_scroll + 1);
        break;

    case skEventType::KeyDown:
        if (m_editRow >= 0) {
            if (ev.button == VK_RETURN || ev.button == VK_DOWN) {
                commitEdit();
                if (m_selRow + 1 < (int)m_rows.size()) {
                    ++m_selRow;
                    if (m_selRow >= m_scroll + visibleRows()) m_scroll = m_selRow - visibleRows() + 1;
                }
            } else if (ev.button == VK_ESCAPE) {
                cancelEdit();
            } else if (ev.button == VK_TAB) {
                navigateEdit(!(GetKeyState(VK_SHIFT) & 0x8000));
            } else if (ev.button == VK_BACK && !m_editBuf.empty()) {
                m_editBuf.pop_back();
            }
        } else {
            // Navigation when not editing
            if (ev.button == VK_UP && m_selRow > 0) {
                --m_selRow; if (m_selRow < m_scroll) m_scroll = m_selRow;
            }
            if (ev.button == VK_DOWN && m_selRow < (int)m_rows.size()-1) {
                ++m_selRow;
                if (m_selRow >= m_scroll + visibleRows()) m_scroll = m_selRow - visibleRows() + 1;
            }
            if (ev.button == VK_RETURN && m_selRow >= 0 && m_selCol >= 0)
                startEdit(m_selRow, m_selCol);
        }
        break;

    case skEventType::KeyChar:
        if (m_editRow >= 0) {
            if (ev.ch >= 32) {
                char buf[5] = {};
                int len = WideCharToMultiByte(CP_UTF8, 0, &ev.ch, 1, buf, 4, nullptr, nullptr);
                m_editBuf.append(buf, len);
            }
        }
        break;

    default: break;
    }
}
