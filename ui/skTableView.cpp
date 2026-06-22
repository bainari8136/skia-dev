#include <Windows.h>
#include "skTableView.h"
#include "skTheme.h"
#include "skTypeface.h"
#include <include/core/SkPaint.h>
#include <include/core/SkFont.h>
#include <include/core/SkRRect.h>
#include <algorithm>

static sk_sp<SkTypeface>& tvTf() {
    static sk_sp<SkTypeface> tf = skGetSystemTypeface();
    return tf;
}

skTableView::skTableView(int tx, int ty, int tw, int th)
    : skWidget(tx, ty, tw, th) {}

void skTableView::addColumn(const std::string& header, float relW) {
    m_cols.push_back({header, relW});
}

void skTableView::addRow(std::vector<std::string> cells) {
    m_rows.push_back({std::move(cells)});
}

void skTableView::clearRows() {
    m_rows.clear(); m_selected = -1; m_hovered = -1; m_scroll = 0;
}

const std::vector<std::string>& skTableView::rowCells(int idx) const {
    static const std::vector<std::string> empty;
    return (idx >= 0 && idx < (int)m_rows.size()) ? m_rows[idx].cells : empty;
}

void skTableView::setOnSelect(std::function<void(int, const std::vector<std::string>&)> fn) {
    m_onSelect = std::move(fn);
}

int skTableView::visibleRows() const {
    return std::max(0, (int)(((float)h - kHeaderH) / kRowH));
}

int skTableView::maxScroll() const {
    return std::max(0, (int)m_rows.size() - visibleRows());
}

int skTableView::rowAt(int py) const {
    int rel = py - y - (int)kHeaderH;
    if (rel < 0) return -1;
    int idx = m_scroll + (int)((float)rel / kRowH);
    return (idx >= 0 && idx < (int)m_rows.size()) ? idx : -1;
}

void skTableView::computeColWidths(float listW, std::vector<float>& out) const {
    out.resize(m_cols.size());
    float sumRel = 0.f; int autoN = 0;
    for (auto& c : m_cols) { if (c.relW > 0.f) sumRel += c.relW; else ++autoN; }
    float used = 0.f;
    for (int i = 0; i < (int)m_cols.size(); ++i) {
        out[i] = m_cols[i].relW > 0.f ? listW * m_cols[i].relW : 0.f;
        used += out[i];
    }
    if (autoN > 0) {
        float autoW = (listW - used) / (float)autoN;
        for (int i = 0; i < (int)m_cols.size(); ++i)
            if (m_cols[i].relW <= 0.f) out[i] = autoW;
    }
}

void skTableView::Paint(SkCanvas* canvas) {
    const auto& th = skGetTheme();
    bool  needsSb = (int)m_rows.size() > visibleRows();
    float listW   = (float)w - (needsSb ? kSbW : 0.f);

    SkRRect rr; rr.setRectXY(SkRect::MakeXYWH((float)x,(float)y,(float)w,(float)h),5.f,5.f);
    SkPaint bg; bg.setAntiAlias(true); bg.setColor(th.inputBg);
    canvas->drawRRect(rr, bg);

    canvas->save();
    canvas->clipRRect(rr, SkClipOp::kIntersect, true);

    std::vector<float> cw;
    computeColWidths(listW, cw);

    SkFont hdrFont(tvTf(), 11.f); hdrFont.setEdging(SkFont::Edging::kAntiAlias);
    SkFont rowFont(tvTf(), 12.f); rowFont.setEdging(SkFont::Edging::kAntiAlias);

    // Header row
    SkPaint hdrBg; hdrBg.setColor(SkColorSetARGB(24,
        SkColorGetR(th.accent), SkColorGetG(th.accent), SkColorGetB(th.accent)));
    canvas->drawRect(SkRect::MakeXYWH((float)x,(float)y,listW,kHeaderH), hdrBg);

    SkPaint hdiv; hdiv.setColor(th.panelBorder);
    canvas->drawLine((float)x,(float)y+kHeaderH,(float)x+listW,(float)y+kHeaderH,hdiv);

    float cx = (float)x;
    for (int c = 0; c < (int)m_cols.size(); ++c) {
        if (c > 0) {
            SkPaint vsep; vsep.setColor(th.panelBorder);
            canvas->drawLine(cx,(float)y+2.f,cx,(float)(y+h)-2.f,vsep);
        }
        SkRect tb; hdrFont.measureText(m_cols[c].header.c_str(), m_cols[c].header.size(), SkTextEncoding::kUTF8, &tb);
        SkPaint tp; tp.setAntiAlias(true); tp.setColor(th.accent);
        canvas->drawString(m_cols[c].header.c_str(),
            cx + 8.f - tb.left(),
            (float)y + (kHeaderH - tb.height()) / 2.f - tb.top(),
            hdrFont, tp);
        cx += cw[c];
    }

    // Data rows
    int vis = visibleRows();
    for (int r = m_scroll; r < (int)m_rows.size() && r < m_scroll + vis; ++r) {
        float ry = (float)y + kHeaderH + (float)(r - m_scroll) * kRowH;
        const auto& row = m_rows[r];

        if (r == m_selected) {
            SkPaint sel; sel.setColor(SkColorSetARGB(42,
                SkColorGetR(th.accent), SkColorGetG(th.accent), SkColorGetB(th.accent)));
            canvas->drawRect(SkRect::MakeXYWH((float)x,ry,listW,kRowH),sel);
        } else if (r == m_hovered) {
            SkPaint hov; hov.setColor(SkColorSetARGB(14,
                SkColorGetR(th.accent), SkColorGetG(th.accent), SkColorGetB(th.accent)));
            canvas->drawRect(SkRect::MakeXYWH((float)x,ry,listW,kRowH),hov);
        }

        SkPaint rdiv; rdiv.setColor(SkColorSetARGB(30,
            SkColorGetR(th.panelBorder), SkColorGetG(th.panelBorder), SkColorGetB(th.panelBorder)));
        canvas->drawLine((float)x+6.f,ry,(float)x+listW-6.f,ry,rdiv);

        float cellX = (float)x;
        for (int c = 0; c < (int)cw.size() && c < (int)row.cells.size(); ++c) {
            canvas->save();
            canvas->clipRect(SkRect::MakeXYWH(cellX+2.f,ry,cw[c]-4.f,kRowH));
            SkRect tb; rowFont.measureText(row.cells[c].c_str(), row.cells[c].size(), SkTextEncoding::kUTF8, &tb);
            SkPaint tp; tp.setAntiAlias(true);
            tp.setColor(r == m_selected ? th.accent : th.textPrimary);
            canvas->drawString(row.cells[c].c_str(),
                cellX + 8.f - tb.left(), ry + (kRowH-tb.height())/2.f - tb.top(),
                rowFont, tp);
            canvas->restore();
            cellX += cw[c];
        }
    }

    // Scrollbar
    if (needsSb) {
        float sbX = (float)x + listW;
        SkPaint sbBg; sbBg.setColor(th.trackBg);
        canvas->drawRect(SkRect::MakeXYWH(sbX,(float)y+kHeaderH,kSbW,(float)h-kHeaderH),sbBg);
        float thumbRatio = (float)vis / (float)m_rows.size();
        float thumbH = ((float)h - kHeaderH) * thumbRatio;
        int   ms     = maxScroll();
        float thumbY = (float)y + kHeaderH + ((float)h - kHeaderH - thumbH) *
                       (ms > 0 ? (float)m_scroll / (float)ms : 0.f);
        SkPaint sbTh; sbTh.setAntiAlias(true); sbTh.setColor(th.inputBorder);
        SkRRect tRR; tRR.setRectXY(SkRect::MakeXYWH(sbX+1.f,thumbY+2.f,kSbW-2.f,thumbH-4.f),3.f,3.f);
        canvas->drawRRect(tRR,sbTh);
    }

    canvas->restore();

    SkPaint border; border.setAntiAlias(true);
    border.setStyle(SkPaint::kStroke_Style);
    border.setStrokeWidth(m_focused ? 1.5f : 1.f);
    border.setColor(m_focused ? th.accent : th.inputBorder);
    canvas->drawRRect(rr, border);
}

void skTableView::OnEvent(const skEvent& ev) {
    switch (ev.type) {
        case skEventType::MouseMove:
            m_hovered = contains(ev.x, ev.y) ? rowAt(ev.y) : -1;
            break;
        case skEventType::MouseDown:
            m_focused = contains(ev.x, ev.y);
            if (!m_focused) break;
            {   int r = rowAt(ev.y);
                if (r >= 0) { m_selected = r; if (m_onSelect) m_onSelect(r, m_rows[r].cells); }
            }
            break;
        case skEventType::MouseWheel:
            if (!contains(ev.x, ev.y)) break;
            if (ev.button > 0) m_scroll = std::max(0, m_scroll - 1);
            else               m_scroll = std::min(maxScroll(), m_scroll + 1);
            break;
        case skEventType::KeyDown:
            if (m_rows.empty()) break;
            if (ev.button == VK_UP && m_selected > 0) {
                --m_selected;
                if (m_selected < m_scroll) m_scroll = m_selected;
                if (m_onSelect) m_onSelect(m_selected, m_rows[m_selected].cells);
            }
            if (ev.button == VK_DOWN && m_selected < (int)m_rows.size()-1) {
                ++m_selected;
                if (m_selected >= m_scroll + visibleRows()) m_scroll = m_selected - visibleRows() + 1;
                if (m_onSelect) m_onSelect(m_selected, m_rows[m_selected].cells);
            }
            break;
        default: break;
    }
}
