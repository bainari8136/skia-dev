#include <Windows.h>
#include "skTextArea.h"
#include "skTypeface.h"
#include "skTheme.h"
#include <include/core/SkPaint.h>
#include <include/core/SkFont.h>
#include <include/core/SkRRect.h>
#include <algorithm>

skTextArea::skTextArea(int ax, int ay, int aw, int ah, std::string placeholder)
    : skWidget(ax, ay, aw, ah), m_placeholder(std::move(placeholder)) {}

// ---------------------------------------------------------------------------
// Text access
// ---------------------------------------------------------------------------

std::string skTextArea::text() const {
    std::string result;
    for (int i = 0; i < (int)m_lines.size(); ++i) {
        if (i > 0) result += '\n';
        result += m_lines[i];
    }
    return result;
}

void skTextArea::setText(const std::string& t) {
    m_lines.clear();
    std::string line;
    for (char c : t) {
        if (c == '\n') { m_lines.push_back(line); line.clear(); }
        else           line += c;
    }
    m_lines.push_back(line);
    m_row = 0; m_col = 0; m_scrollRow = 0;
}

// ---------------------------------------------------------------------------
// Focus / tick
// ---------------------------------------------------------------------------

void skTextArea::onFocusGained() {
    m_focused       = true;
    m_cursorVisible = true;
    m_blinkCount    = 0;
}

void skTextArea::onFocusLost() {
    m_focused = false;
}

void skTextArea::onTick() {
    if (!m_focused) return;
    if (++m_blinkCount >= 5) {
        m_blinkCount    = 0;
        m_cursorVisible = !m_cursorVisible;
    }
}

// ---------------------------------------------------------------------------
// Internal edit helpers
// ---------------------------------------------------------------------------

void skTextArea::insertChar(char ch) {
    m_lines[m_row].insert(m_col, 1, ch);
    ++m_col;
}

void skTextArea::backspace() {
    if (m_col > 0) {
        m_lines[m_row].erase(m_col - 1, 1);
        --m_col;
    } else if (m_row > 0) {
        m_col = (int)m_lines[m_row - 1].size();
        m_lines[m_row - 1] += m_lines[m_row];
        m_lines.erase(m_lines.begin() + m_row);
        --m_row;
        scrollToCursor();
    }
}

void skTextArea::deleteChar() {
    if (m_col < (int)m_lines[m_row].size()) {
        m_lines[m_row].erase(m_col, 1);
    } else if (m_row < (int)m_lines.size() - 1) {
        m_lines[m_row] += m_lines[m_row + 1];
        m_lines.erase(m_lines.begin() + m_row + 1);
    }
}

void skTextArea::moveLeft() {
    if (m_col > 0) --m_col;
    else if (m_row > 0) { --m_row; m_col = (int)m_lines[m_row].size(); scrollToCursor(); }
}

void skTextArea::moveRight() {
    if (m_col < (int)m_lines[m_row].size()) ++m_col;
    else if (m_row < (int)m_lines.size() - 1) { ++m_row; m_col = 0; scrollToCursor(); }
}

void skTextArea::moveUp() {
    if (m_row > 0) {
        --m_row;
        m_col = std::min(m_col, (int)m_lines[m_row].size());
        scrollToCursor();
    }
}

void skTextArea::moveDown() {
    if (m_row < (int)m_lines.size() - 1) {
        ++m_row;
        m_col = std::min(m_col, (int)m_lines[m_row].size());
        scrollToCursor();
    }
}

void skTextArea::scrollToCursor() {
    int vis = visibleRows();
    if (m_row < m_scrollRow)              m_scrollRow = m_row;
    if (m_row >= m_scrollRow + vis)       m_scrollRow = m_row - vis + 1;
    m_scrollRow = std::max(0, m_scrollRow);
}

// ---------------------------------------------------------------------------
// Paint
// ---------------------------------------------------------------------------

void skTextArea::Paint(SkCanvas* canvas) {
    const auto& th = skGetTheme();

    SkRRect rr;
    rr.setRectXY(SkRect::MakeXYWH((float)x, (float)y, (float)w, (float)h), 6.f, 6.f);

    SkPaint bg;
    bg.setAntiAlias(true);
    bg.setColor(th.inputBg);
    canvas->drawRRect(rr, bg);

    canvas->save();
    canvas->clipRRect(rr, SkClipOp::kIntersect, true);

    bool empty = (m_lines.size() == 1 && m_lines[0].empty());

    static sk_sp<SkTypeface> s_tf = skGetSystemTypeface();
    SkFont font(s_tf, 13.f);
    font.setEdging(SkFont::Edging::kAntiAlias);

    if (empty && !m_focused) {
        SkPaint pp;
        pp.setAntiAlias(true);
        pp.setColor(th.textPlaceholder);
        canvas->drawString(m_placeholder.c_str(),
            (float)x + kPad,
            (float)y + kPad + 13.f,
            font, pp);
    } else {
        int vis = visibleRows();

        for (int i = m_scrollRow; i < (int)m_lines.size() && i <= m_scrollRow + vis; ++i) {
            float lineTop = (float)y + kPad + (float)(i - m_scrollRow) * kLineH;
            float baseline = lineTop + 13.f;

            if (!m_lines[i].empty()) {
                SkPaint tp;
                tp.setAntiAlias(true);
                tp.setColor(th.textPrimary);
                canvas->drawString(m_lines[i].c_str(), (float)x + kPad, baseline, font, tp);
            }

            // Cursor
            if (m_focused && m_cursorVisible && i == m_row) {
                float cx = (float)x + kPad;
                if (m_col > 0 && !m_lines[i].empty()) {
                    int safeCol = std::min(m_col, (int)m_lines[i].size());
                    cx += font.measureText(m_lines[i].c_str(), (size_t)safeCol,
                                          SkTextEncoding::kUTF8, nullptr);
                }
                SkPaint cp;
                cp.setAntiAlias(true);
                cp.setColor(th.accent);
                cp.setStrokeWidth(1.5f);
                canvas->drawLine(cx, lineTop + 2.f, cx, lineTop + kLineH - 2.f, cp);
            }
        }
    }

    canvas->restore();

    // Scrollbar
    int total = (int)m_lines.size();
    int vis   = visibleRows();
    if (total > vis) {
        float sbX   = (float)(x + w) - kSbW - 2.f;
        float sbH   = (float)h - 8.f;
        float sbY   = (float)y + 4.f;
        float ratio = (float)vis / (float)total;
        float tH    = sbH * ratio;
        int   ms    = total - vis;
        float tY    = sbY + (sbH - tH) * ((float)m_scrollRow / (float)ms);

        SkPaint sbP;
        sbP.setAntiAlias(true);
        sbP.setColor(th.inputBorder);
        SkRRect tRR;
        tRR.setRectXY(SkRect::MakeXYWH(sbX, tY, kSbW - 2.f, tH), 3.f, 3.f);
        canvas->drawRRect(tRR, sbP);
    }

    // Border
    SkPaint brd;
    brd.setAntiAlias(true);
    brd.setStyle(SkPaint::kStroke_Style);
    brd.setStrokeWidth(m_focused ? 2.f : 1.f);
    brd.setColor(m_focused ? th.accent : th.inputBorder);
    canvas->drawRRect(rr, brd);
}

// ---------------------------------------------------------------------------
// Events
// ---------------------------------------------------------------------------

void skTextArea::OnEvent(const skEvent& ev) {
    switch (ev.type) {
        case skEventType::MouseDown: {
            if (!contains(ev.x, ev.y)) break;
            // Set row from y
            float relY  = (float)(ev.y - y) - kPad;
            int clickRow = m_scrollRow + (int)(relY / kLineH);
            m_row = std::max(0, std::min((int)m_lines.size() - 1, clickRow));

            // Set col from x (binary search via linear scan)
            float relX = (float)(ev.x - x) - kPad;
            static sk_sp<SkTypeface> s_tf2 = skGetSystemTypeface();
            SkFont font(s_tf2, 13.f);
            const auto& line = m_lines[m_row];
            m_col = (int)line.size();
            for (int c = 0; c < (int)line.size(); ++c) {
                float adv  = font.measureText(line.c_str(), (size_t)c,   SkTextEncoding::kUTF8, nullptr);
                float advN = font.measureText(line.c_str(), (size_t)c+1, SkTextEncoding::kUTF8, nullptr);
                if (relX < (adv + advN) / 2.f) { m_col = c; break; }
            }
            m_cursorVisible = true;
            m_blinkCount    = 0;
            break;
        }

        case skEventType::MouseWheel: {
            if (!contains(ev.x, ev.y)) break;
            int delta = (ev.button > 0) ? -3 : 3;
            m_scrollRow = std::max(0, std::min(m_scrollRow + delta,
                                               std::max(0, (int)m_lines.size() - visibleRows())));
            break;
        }

        case skEventType::KeyChar: {
            wchar_t ch = ev.ch;
            if (ch == 13) {
                // Enter: split line
                std::string rem = m_lines[m_row].substr(m_col);
                m_lines[m_row] = m_lines[m_row].substr(0, m_col);
                m_lines.insert(m_lines.begin() + m_row + 1, rem);
                ++m_row;
                m_col = 0;
                scrollToCursor();
            } else if (ch == 8) {
                backspace();
            } else if (ch >= 32) {
                insertChar((char)ch);
            }
            m_cursorVisible = true;
            m_blinkCount    = 0;
            if (m_onChange) m_onChange(text());
            break;
        }

        case skEventType::KeyDown:
            switch (ev.button) {
                case VK_LEFT:   moveLeft();  break;
                case VK_RIGHT:  moveRight(); break;
                case VK_UP:     moveUp();    break;
                case VK_DOWN:   moveDown();  break;
                case VK_HOME:   m_col = 0;  break;
                case VK_END:    m_col = (int)m_lines[m_row].size(); break;
                case VK_DELETE: deleteChar(); if (m_onChange) m_onChange(text()); break;
            }
            m_cursorVisible = true;
            m_blinkCount    = 0;
            break;

        default: break;
    }
}
