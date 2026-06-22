#include <Windows.h>
#include "skPropertyGrid.h"
#include "skTheme.h"
#include "skTypeface.h"
#include <include/core/SkPaint.h>
#include <include/core/SkFont.h>
#include <include/core/SkRRect.h>
#include <algorithm>

static sk_sp<SkTypeface>& pgTf() {
    static sk_sp<SkTypeface> tf = skGetSystemTypeface();
    return tf;
}

skPropertyGrid::skPropertyGrid(int gx, int gy, int gw, int gh)
    : skWidget(gx, gy, gw, gh) {}

void skPropertyGrid::addProperty(const std::string& key, const std::string& val, bool editable) {
    m_props.push_back({key, val, editable});
}

void skPropertyGrid::setProperty(int idx, const std::string& val) {
    if (idx >= 0 && idx < (int)m_props.size()) m_props[idx].val = val;
}

void skPropertyGrid::clearProperties() {
    m_props.clear(); m_selected = -1; m_scroll = 0; m_editing = false;
}

std::string skPropertyGrid::value(int idx) const {
    return (idx >= 0 && idx < (int)m_props.size()) ? m_props[idx].val : "";
}

void skPropertyGrid::setOnChange(std::function<void(int, const std::string&, const std::string&)> fn) {
    m_onChange = std::move(fn);
}

int skPropertyGrid::rowAt(int py) const {
    int rel = py - y;
    if (rel < 0) return -1;
    int idx = m_scroll + (int)((float)rel / kRowH);
    return (idx >= 0 && idx < (int)m_props.size()) ? idx : -1;
}

void skPropertyGrid::Paint(SkCanvas* canvas) {
    const auto& th = skGetTheme();
    float keyW = (float)w * kKeyFr;

    SkRRect rr; rr.setRectXY(SkRect::MakeXYWH((float)x,(float)y,(float)w,(float)h),5.f,5.f);
    SkPaint bg; bg.setAntiAlias(true); bg.setColor(th.inputBg);
    canvas->drawRRect(rr, bg);

    canvas->save();
    canvas->clipRRect(rr, SkClipOp::kIntersect, true);

    SkFont font(pgTf(), 12.f); font.setEdging(SkFont::Edging::kAntiAlias);

    int vis = visibleRows();
    for (int i = m_scroll; i < (int)m_props.size() && i < m_scroll + vis; ++i) {
        float ry = (float)y + (float)(i - m_scroll) * kRowH;
        const auto& p = m_props[i];
        bool active = (i == m_selected);

        // Key column background
        SkPaint kbg; kbg.setColor(SkColorSetARGB(12, 100, 100, 100));
        canvas->drawRect(SkRect::MakeXYWH((float)x, ry, keyW, kRowH), kbg);

        // Value column selection
        if (active) {
            SkPaint sel; sel.setColor(SkColorSetARGB(36,
                SkColorGetR(th.accent), SkColorGetG(th.accent), SkColorGetB(th.accent)));
            canvas->drawRect(SkRect::MakeXYWH((float)x + keyW, ry, (float)w - keyW, kRowH), sel);
        }

        // Row divider
        SkPaint div; div.setColor(th.panelBorder);
        canvas->drawLine((float)x, ry, (float)(x+w), ry, div);

        // Key text
        SkRect ktb;
        font.measureText(p.key.c_str(), p.key.size(), SkTextEncoding::kUTF8, &ktb);
        SkPaint ktp; ktp.setAntiAlias(true); ktp.setColor(th.textSecondary);
        canvas->drawString(p.key.c_str(),
            (float)x + 8.f - ktb.left(),
            ry + (kRowH - ktb.height()) / 2.f - ktb.top(), font, ktp);

        // Value text (clipped)
        canvas->save();
        canvas->clipRect(SkRect::MakeXYWH((float)x + keyW + 4.f, ry, (float)w - keyW - 8.f, kRowH));
        SkRect vtb;
        font.measureText(p.val.c_str(), p.val.size(), SkTextEncoding::kUTF8, &vtb);
        SkPaint vtp; vtp.setAntiAlias(true);
        vtp.setColor(active ? th.accent : th.textPrimary);
        canvas->drawString(p.val.c_str(),
            (float)x + keyW + 8.f - vtb.left(),
            ry + (kRowH - vtb.height()) / 2.f - vtb.top(), font, vtp);
        canvas->restore();

        // Edit cursor
        if (active && m_editing && p.editable) {
            float cx_ = (float)x + keyW + 8.f + vtb.width() + 1.f;
            SkPaint cp; cp.setColor(th.accent);
            canvas->drawLine(cx_, ry + 4.f, cx_, ry + kRowH - 4.f, cp);
        }
    }

    // Column divider
    SkPaint cdiv; cdiv.setColor(th.panelBorder);
    canvas->drawLine((float)x + keyW, (float)y, (float)x + keyW, (float)(y+h), cdiv);

    canvas->restore();

    SkPaint border; border.setAntiAlias(true);
    border.setStyle(SkPaint::kStroke_Style);
    border.setStrokeWidth(m_editing ? 1.5f : 1.f);
    border.setColor(m_editing ? th.accent : th.inputBorder);
    canvas->drawRRect(rr, border);
}

void skPropertyGrid::OnEvent(const skEvent& ev) {
    switch (ev.type) {
        case skEventType::MouseDown:
            if (!contains(ev.x, ev.y)) { m_editing = false; break; }
            {   int r = rowAt(ev.y);
                if (r >= 0) {
                    if (r == m_selected && m_props[r].editable) m_editing = !m_editing;
                    else { m_selected = r; m_editing = false; }
                }
            }
            break;
        case skEventType::MouseWheel:
            if (!contains(ev.x, ev.y)) break;
            if (ev.button > 0) m_scroll = std::max(0, m_scroll - 1);
            else               m_scroll = std::min(maxScroll(), m_scroll + 1);
            break;
        case skEventType::KeyChar:
            if (m_selected < 0 || !m_editing) break;
            if (!m_props[m_selected].editable) break;
            if (ev.ch >= 32) {
                m_props[m_selected].val += (char)ev.ch;
                if (m_onChange) m_onChange(m_selected, m_props[m_selected].key, m_props[m_selected].val);
            }
            break;
        case skEventType::KeyDown:
            if (m_selected < 0) break;
            switch (ev.button) {
                case VK_BACK:
                    if (m_editing && !m_props[m_selected].val.empty()) {
                        m_props[m_selected].val.pop_back();
                        if (m_onChange) m_onChange(m_selected, m_props[m_selected].key, m_props[m_selected].val);
                    }
                    break;
                case VK_ESCAPE: m_editing = false; break;
                case VK_RETURN:
                    if (m_props[m_selected].editable) m_editing = !m_editing;
                    break;
                case VK_UP:
                    if (m_selected > 0) { --m_selected; m_editing = false; }
                    break;
                case VK_DOWN:
                    if (m_selected < (int)m_props.size()-1) { ++m_selected; m_editing = false; }
                    break;
            }
            break;
        default: break;
    }
}
