#include "skDropdown.h"
#include "skTypeface.h"
#include "skTheme.h"
#include <include/core/SkPaint.h>
#include <include/core/SkFont.h>
#include <include/core/SkRRect.h>
#include <include/core/SkPath.h>

skDropdown::skDropdown(int dx, int dy, int dw, int dh) : skWidget(dx, dy, dw, dh) {}

void skDropdown::addOption(std::string option) { m_options.push_back(std::move(option)); }

const std::string& skDropdown::selectedText() const {
    static const std::string empty;
    return m_options.empty() ? empty : m_options[m_selected];
}

SkRect skDropdown::listRect() const {
    return SkRect::MakeXYWH((float)x, (float)y + (float)h,
                             (float)w, kRowH * (float)m_options.size());
}

void skDropdown::Paint(SkCanvas* canvas) {
    canvas->save();
    const auto& th = skGetTheme();

    // ---- Closed button ----
    SkPaint bg;
    bg.setAntiAlias(true);
    bg.setColor(th.inputBg);
    SkRRect rr;
    rr.setRectXY(SkRect::MakeXYWH((float)x, (float)y, (float)w, (float)h), 6.f, 6.f);
    canvas->drawRRect(rr, bg);

    SkPaint border;
    border.setAntiAlias(true);
    border.setStyle(SkPaint::kStroke_Style);
    border.setStrokeWidth(m_open ? 2.f : 1.f);
    border.setColor(m_open ? th.accent : th.inputBorder);
    canvas->drawRRect(rr, border);

    // Selected text
    static sk_sp<SkTypeface> s_tf = skGetSystemTypeface();
    SkFont font(s_tf, 15.f);
    font.setEdging(SkFont::Edging::kAntiAlias);

    const std::string& label = selectedText();
    SkRect lb;
    font.measureText(label.c_str(), label.size(), SkTextEncoding::kUTF8, &lb);
    float ty = (float)y + ((float)h - lb.height()) / 2.f - lb.top();

    SkPaint textPaint;
    textPaint.setAntiAlias(true);
    textPaint.setColor(th.textPrimary);
    canvas->drawString(label.c_str(), (float)x + 10.f - lb.left(), ty, font, textPaint);

    // Chevron ▾
    float cx = (float)x + (float)w - 18.f;
    float cy = (float)y + (float)h / 2.f;
    SkPaint chev;
    chev.setAntiAlias(true);
    chev.setStyle(SkPaint::kStroke_Style);
    chev.setStrokeWidth(1.8f);
    chev.setStrokeCap(SkPaint::kRound_Cap);
    chev.setColor(th.textSecondary);
    SkPath chevPath;
    if (m_open) {
        chevPath.moveTo(cx - 5.f, cy + 3.f);
        chevPath.lineTo(cx,       cy - 2.f);
        chevPath.lineTo(cx + 5.f, cy + 3.f);
    } else {
        chevPath.moveTo(cx - 5.f, cy - 2.f);
        chevPath.lineTo(cx,       cy + 3.f);
        chevPath.lineTo(cx + 5.f, cy - 2.f);
    }
    canvas->drawPath(chevPath, chev);

    // ---- Open dropdown list ----
    if (m_open && !m_options.empty()) {
        SkRect lr = listRect();

        SkPaint listBg;
        listBg.setAntiAlias(true);
        listBg.setColor(th.panelBg);
        SkRRect listRR;
        listRR.setRectXY(lr, 6.f, 6.f);
        canvas->drawRRect(listRR, listBg);

        SkPaint listBorder;
        listBorder.setAntiAlias(true);
        listBorder.setStyle(SkPaint::kStroke_Style);
        listBorder.setStrokeWidth(1.f);
        listBorder.setColor(th.panelBorder);
        canvas->drawRRect(listRR, listBorder);

        for (int i = 0; i < (int)m_options.size(); ++i) {
            float ry = lr.top() + i * kRowH;

            if (i == m_hovered || i == m_selected) {
                SkPaint rowBg;
                rowBg.setAntiAlias(true);
                rowBg.setColor(i == m_hovered
                    ? SkColorSetARGB(25, SkColorGetR(th.accent), SkColorGetG(th.accent), SkColorGetB(th.accent))
                    : SkColorSetARGB(15, SkColorGetR(th.accent), SkColorGetG(th.accent), SkColorGetB(th.accent)));
                canvas->drawRect(SkRect::MakeLTRB(lr.left()+1, ry, lr.right()-1, ry + kRowH), rowBg);
            }

            SkRect ob;
            font.measureText(m_options[i].c_str(), m_options[i].size(), SkTextEncoding::kUTF8, &ob);
            float oty = ry + (kRowH - ob.height()) / 2.f - ob.top();

            SkPaint op;
            op.setAntiAlias(true);
            op.setColor(i == m_selected ? th.accent : th.textPrimary);
            canvas->drawString(m_options[i].c_str(), lr.left() + 10.f - ob.left(), oty, font, op);
        }
    }

    canvas->restore();
}

void skDropdown::OnEvent(const skEvent& event) {
    if (event.type == skEventType::MouseMove && m_open) {
        SkRect lr = listRect();
        m_hovered = -1;
        if (lr.contains((float)event.x, (float)event.y)) {
            m_hovered = (int)((event.y - lr.top()) / kRowH);
        }
    }
}

bool skDropdown::handleEvent(const skEvent& ev) {
    if (ev.type == skEventType::MouseDown) {
        // Click on the closed button → toggle open
        if (contains(ev.x, ev.y)) {
            m_open = !m_open;
            m_hovered = -1;
            return true;
        }
        // Click inside the open list → select option
        if (m_open) {
            SkRect lr = listRect();
            if (lr.contains((float)ev.x, (float)ev.y)) {
                int idx = (int)((ev.y - lr.top()) / kRowH);
                if (idx >= 0 && idx < (int)m_options.size()) {
                    m_selected = idx;
                    if (m_onChange) m_onChange(idx, m_options[idx]);
                }
                m_open = false;
                return true;
            }
            // Click outside → close
            m_open = false;
            return false;
        }
    }
    return false;
}

std::shared_ptr<skDropdown> skDropdown::make(int dx, int dy, int dw, int dh) {
    return std::make_shared<skDropdown>(dx, dy, dw, dh);
}
std::shared_ptr<skDropdown> skDropdown::onChange(std::function<void(int, const std::string&)> cb) {
    m_onChange = std::move(cb); return shared_this();
}
std::shared_ptr<skDropdown> skDropdown::pos(int px, int py)  { x = px; y = py; return shared_this(); }
std::shared_ptr<skDropdown> skDropdown::size(int pw, int ph) { w = pw; h = ph; return shared_this(); }
