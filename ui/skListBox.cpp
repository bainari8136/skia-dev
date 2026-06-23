#include <Windows.h>
#include "skListBox.h"
#include "skTypeface.h"
#include "skTheme.h"
#include <include/core/SkPaint.h>
#include <include/core/SkFont.h>
#include <include/core/SkRRect.h>
#include <algorithm>

skListBox::skListBox(int lx, int ly, int lw, int lh)
    : skWidget(lx, ly, lw, lh) {}

std::shared_ptr<skListBox> skListBox::make(int lx, int ly, int lw, int lh) {
    return std::make_shared<skListBox>(lx, ly, lw, lh);
}
std::shared_ptr<skListBox> skListBox::onChange(std::function<void(int, const std::string&)> cb) {
    m_onChange = std::move(cb); return shared_this();
}
std::shared_ptr<skListBox> skListBox::pos(int px, int py)  { x = px; y = py; return shared_this(); }
std::shared_ptr<skListBox> skListBox::size(int pw, int ph) { w = pw; h = ph; return shared_this(); }

void skListBox::addItem(std::string item)  { m_items.push_back(std::move(item)); }
void skListBox::clearItems()               { m_items.clear(); m_selected = -1; m_hovered = -1; m_scroll = 0; }

int skListBox::maxScroll() const {
    return std::max(0, (int)m_items.size() - visibleCount());
}

const std::string& skListBox::selectedText() const {
    static const std::string empty;
    return (m_selected >= 0 && m_selected < (int)m_items.size()) ? m_items[m_selected] : empty;
}

void skListBox::Paint(SkCanvas* canvas) {
    const auto& th = skGetTheme();

    SkRRect rr;
    rr.setRectXY(SkRect::MakeXYWH((float)x, (float)y, (float)w, (float)h), 6.f, 6.f);

    // Background
    SkPaint bg;
    bg.setAntiAlias(true);
    bg.setColor(th.inputBg);
    canvas->drawRRect(rr, bg);

    // Clip items to the rounded bounds
    canvas->save();
    canvas->clipRRect(rr, SkClipOp::kIntersect, true);

    const bool needsSb = (int)m_items.size() > visibleCount();
    const float listW  = (float)w - (needsSb ? kSbW : 0.f);

    static sk_sp<SkTypeface> s_tf = skGetSystemTypeface();
    SkFont font(s_tf, 14.f);
    font.setEdging(SkFont::Edging::kAntiAlias);

    int visible = visibleCount();
    for (int i = m_scroll; i < (int)m_items.size() && i < m_scroll + visible; ++i) {
        float ry = (float)y + (float)(i - m_scroll) * kRowH;

        if (i == m_selected) {
            SkPaint sel;
            sel.setColor(SkColorSetARGB(45,
                SkColorGetR(th.accent), SkColorGetG(th.accent), SkColorGetB(th.accent)));
            canvas->drawRect(SkRect::MakeLTRB((float)x, ry, (float)x + listW, ry + kRowH), sel);
        } else if (i == m_hovered) {
            SkPaint hov;
            hov.setColor(SkColorSetARGB(18,
                SkColorGetR(th.accent), SkColorGetG(th.accent), SkColorGetB(th.accent)));
            canvas->drawRect(SkRect::MakeLTRB((float)x, ry, (float)x + listW, ry + kRowH), hov);
        }

        // Row divider (subtle)
        if (i > m_scroll) {
            SkPaint div;
            div.setColor(SkColorSetARGB(40,
                SkColorGetR(th.panelBorder), SkColorGetG(th.panelBorder), SkColorGetB(th.panelBorder)));
            canvas->drawLine((float)x + 8, ry, (float)x + listW - 8, ry, div);
        }

        SkRect tb;
        font.measureText(m_items[i].c_str(), m_items[i].size(), SkTextEncoding::kUTF8, &tb);
        float ty = ry + (kRowH - tb.height()) / 2.f - tb.top();

        SkPaint tp;
        tp.setAntiAlias(true);
        tp.setColor(i == m_selected ? th.accent : th.textPrimary);
        canvas->drawString(m_items[i].c_str(), (float)x + 10.f - tb.left(), ty, font, tp);
    }

    // Scrollbar
    if (needsSb) {
        float sbX = (float)x + listW;
        SkPaint sbBg;
        sbBg.setColor(th.trackBg);
        canvas->drawRect(SkRect::MakeXYWH(sbX, (float)y, kSbW, (float)h), sbBg);

        int ms = maxScroll();
        float thumbRatio = (float)visible / (float)m_items.size();
        float thumbH  = (float)h * thumbRatio;
        float thumbY  = (float)y + ((float)h - thumbH) * ((ms > 0) ? (float)m_scroll / (float)ms : 0.f);

        SkPaint sbTh;
        sbTh.setAntiAlias(true);
        sbTh.setColor(th.inputBorder);
        SkRRect tRR;
        tRR.setRectXY(SkRect::MakeXYWH(sbX + 1.f, thumbY + 2.f, kSbW - 2.f, thumbH - 4.f), 3.f, 3.f);
        canvas->drawRRect(tRR, sbTh);
    }

    canvas->restore();

    // Border on top so items don't overdraw it
    SkPaint border;
    border.setAntiAlias(true);
    border.setStyle(SkPaint::kStroke_Style);
    border.setStrokeWidth(m_focused ? 2.f : 1.f);
    border.setColor(m_focused ? th.accent : th.inputBorder);
    canvas->drawRRect(rr, border);
}

void skListBox::OnEvent(const skEvent& event) {
    switch (event.type) {
        case skEventType::MouseDown: {
            if (!contains(event.x, event.y)) break;
            int vi  = (int)((event.y - y) / kRowH);
            int idx = m_scroll + vi;
            if (idx >= 0 && idx < (int)m_items.size()) {
                m_selected = idx;
                if (m_onChange) m_onChange(idx, m_items[idx]);
            }
            break;
        }
        case skEventType::MouseMove: {
            if (!contains(event.x, event.y)) { m_hovered = -1; break; }
            int vi  = (int)((event.y - y) / kRowH);
            int idx = m_scroll + vi;
            m_hovered = (idx >= 0 && idx < (int)m_items.size()) ? idx : -1;
            break;
        }
        case skEventType::MouseWheel: {
            int delta = event.button;
            if (delta > 0) m_scroll = std::max(0, m_scroll - 1);
            else           m_scroll = std::min(maxScroll(), m_scroll + 1);
            break;
        }
        case skEventType::KeyDown:
            if (m_items.empty()) break;
            switch (event.button) {
                case VK_UP:
                    if (m_selected > 0) {
                        --m_selected;
                        if (m_selected < m_scroll) m_scroll = m_selected;
                        if (m_onChange) m_onChange(m_selected, m_items[m_selected]);
                    }
                    break;
                case VK_DOWN:
                    if (m_selected < (int)m_items.size() - 1) {
                        ++m_selected;
                        if (m_selected >= m_scroll + visibleCount())
                            m_scroll = m_selected - visibleCount() + 1;
                        if (m_onChange) m_onChange(m_selected, m_items[m_selected]);
                    }
                    break;
            }
            break;
        default: break;
    }
}
