#include <Windows.h>
#include "skTreeView.h"
#include "skTypeface.h"
#include "skTheme.h"
#include <include/core/SkPaint.h>
#include <include/core/SkFont.h>
#include <include/core/SkPath.h>
#include <include/core/SkRRect.h>
#include <algorithm>

skTreeView::skTreeView(int tx, int ty, int tw, int th)
    : skWidget(tx, ty, tw, th) {}

skTreeNode* skTreeView::addRoot(const std::string& label) {
    m_roots.push_back(std::make_unique<skTreeNode>());
    m_roots.back()->label = label;
    return m_roots.back().get();
}

skTreeNode* skTreeView::addChild(skTreeNode* parent, const std::string& label) {
    parent->children.push_back(std::make_unique<skTreeNode>());
    parent->children.back()->label = label;
    return parent->children.back().get();
}

void skTreeView::buildFlat(std::vector<FlatNode>& out,
                            std::vector<std::unique_ptr<skTreeNode>>& nodes, int depth) {
    for (auto& n : nodes) {
        out.push_back({n.get(), depth});
        if (n->expanded && !n->children.empty())
            buildFlat(out, n->children, depth + 1);
    }
}

std::vector<skTreeView::FlatNode> skTreeView::getFlat() {
    std::vector<FlatNode> flat;
    buildFlat(flat, m_roots, 0);
    return flat;
}

// ---------------------------------------------------------------------------
// Paint
// ---------------------------------------------------------------------------

void skTreeView::Paint(SkCanvas* canvas) {
    const auto& th = skGetTheme();

    SkRect bounds = SkRect::MakeXYWH((float)x, (float)y, (float)w, (float)h);
    SkRRect rr;
    rr.setRectXY(bounds, 6.f, 6.f);

    SkPaint bg;
    bg.setAntiAlias(true);
    bg.setColor(th.inputBg);
    canvas->drawRRect(rr, bg);

    auto flat = getFlat();
    int  vis  = visibleRows();

    static sk_sp<SkTypeface> s_tf = skGetSystemTypeface();
    SkFont font(s_tf, 13.f);
    font.setEdging(SkFont::Edging::kAntiAlias);

    canvas->save();
    canvas->clipRRect(rr, SkClipOp::kIntersect, true);

    for (int i = m_scrollOffset; i < (int)flat.size() && i <= m_scrollOffset + vis; ++i) {
        auto& fn   = flat[i];
        float rowY = (float)y + (float)(i - m_scrollOffset) * kRowH;

        // Row highlight
        if (i == m_selectedRow) {
            SkPaint sel;
            sel.setColor(SkColorSetARGB(30,
                SkColorGetR(th.accent), SkColorGetG(th.accent), SkColorGetB(th.accent)));
            canvas->drawRect(SkRect::MakeLTRB((float)x, rowY, (float)(x + w), rowY + kRowH), sel);
        } else if (i == m_hoverRow) {
            SkPaint hov;
            hov.setColor(SkColorSetARGB(14, 128, 128, 128));
            canvas->drawRect(SkRect::MakeLTRB((float)x, rowY, (float)(x + w), rowY + kRowH), hov);
        }

        float lx   = (float)x + 6.f + fn.depth * kIndentW;
        float midY = rowY + kRowH / 2.f;

        // Chevron (branch) or dot (leaf)
        const float sz = 4.f;
        if (!fn.node->children.empty()) {
            SkPath chev;
            if (fn.node->expanded) {
                chev.moveTo(lx,          midY - sz * 0.5f);
                chev.lineTo(lx + sz * 2, midY - sz * 0.5f);
                chev.lineTo(lx + sz,     midY + sz * 0.5f);
            } else {
                chev.moveTo(lx,          midY - sz);
                chev.lineTo(lx + sz,     midY);
                chev.lineTo(lx,          midY + sz);
            }
            chev.close();
            SkPaint cp;
            cp.setAntiAlias(true);
            cp.setColor(fn.node->expanded ? th.accent : th.textSecondary);
            canvas->drawPath(chev, cp);
        } else {
            SkPaint dp;
            dp.setAntiAlias(true);
            dp.setColor(SkColorSetARGB(120,
                SkColorGetR(th.textSecondary), SkColorGetG(th.textSecondary), SkColorGetB(th.textSecondary)));
            canvas->drawCircle(lx + sz * 0.5f + 1.f, midY, 2.f, dp);
        }

        // Label
        float textX = lx + kChevW;
        SkRect lb;
        font.measureText(fn.node->label.c_str(), fn.node->label.size(),
                         SkTextEncoding::kUTF8, &lb);

        SkPaint lp;
        lp.setAntiAlias(true);
        lp.setColor(i == m_selectedRow ? th.accent : th.textPrimary);
        canvas->drawString(fn.node->label.c_str(),
            textX - lb.left(),
            midY - lb.height() / 2.f - lb.top(),
            font, lp);
    }

    // Scrollbar
    int total = (int)flat.size();
    if (total > vis) {
        float sbX   = (float)(x + w) - 8.f;
        float sbH   = (float)h - 8.f;
        float ratio = (float)vis / (float)total;
        float tH    = std::max(14.f, sbH * ratio);
        int   ms    = total - vis;
        float tY    = (float)y + 4.f + (sbH - tH) * ((float)m_scrollOffset / (float)ms);

        SkPaint sbt;
        sbt.setAntiAlias(true);
        sbt.setColor(th.inputBorder);
        SkRRect tRR;
        tRR.setRectXY(SkRect::MakeXYWH(sbX, tY, 6.f, tH), 3.f, 3.f);
        canvas->drawRRect(tRR, sbt);
    }

    canvas->restore();

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

void skTreeView::OnEvent(const skEvent& ev) {
    auto flat = getFlat();
    int  vis  = visibleRows();

    switch (ev.type) {
        case skEventType::MouseMove: {
            if (!contains(ev.x, ev.y)) { m_hoverRow = -1; break; }
            int row    = m_scrollOffset + (int)((float)(ev.y - y) / kRowH);
            m_hoverRow = (row >= 0 && row < (int)flat.size()) ? row : -1;
            break;
        }

        case skEventType::MouseDown: {
            if (!contains(ev.x, ev.y)) break;
            int row = m_scrollOffset + (int)((float)(ev.y - y) / kRowH);
            if (row < 0 || row >= (int)flat.size()) break;

            auto& fn  = flat[row];
            float lx  = (float)x + 6.f + fn.depth * kIndentW;

            if (!fn.node->children.empty() && (float)ev.x <= lx + kChevW) {
                // Toggle expansion
                fn.node->expanded = !fn.node->expanded;
                // Clamp scroll in case tree got shorter
                auto newFlat = getFlat();
                m_scrollOffset = std::max(0, std::min(m_scrollOffset,
                                          std::max(0, (int)newFlat.size() - vis)));
                if (m_selectedRow >= (int)newFlat.size())
                    m_selectedRow = -1;
            } else {
                m_selectedRow = row;
                if (m_onSelect) m_onSelect(fn.node->label);
            }
            break;
        }

        case skEventType::MouseWheel: {
            int ms = std::max(0, (int)flat.size() - vis);
            m_scrollOffset += (ev.button > 0) ? -3 : 3;
            m_scrollOffset  = std::max(0, std::min(m_scrollOffset, ms));
            break;
        }

        case skEventType::KeyDown: {
            if (flat.empty()) break;
            int ms = std::max(0, (int)flat.size() - vis);

            if (ev.button == VK_UP) {
                if (m_selectedRow > 0) --m_selectedRow;
                else if (m_selectedRow < 0) m_selectedRow = 0;
                if (m_selectedRow < m_scrollOffset)
                    m_scrollOffset = m_selectedRow;

            } else if (ev.button == VK_DOWN) {
                if (m_selectedRow < (int)flat.size() - 1) ++m_selectedRow;
                else if (m_selectedRow < 0) m_selectedRow = 0;
                if (m_selectedRow >= m_scrollOffset + vis)
                    m_scrollOffset = std::min(ms, m_selectedRow - vis + 1);

            } else if (ev.button == VK_LEFT) {
                if (m_selectedRow >= 0 && m_selectedRow < (int)flat.size()) {
                    auto* node = flat[m_selectedRow].node;
                    if (!node->children.empty() && node->expanded)
                        node->expanded = false;
                }

            } else if (ev.button == VK_RIGHT) {
                if (m_selectedRow >= 0 && m_selectedRow < (int)flat.size()) {
                    auto* node = flat[m_selectedRow].node;
                    if (!node->children.empty() && !node->expanded)
                        node->expanded = true;
                }

            } else if (ev.button == VK_RETURN) {
                if (m_selectedRow >= 0 && m_selectedRow < (int)flat.size()) {
                    auto* node = flat[m_selectedRow].node;
                    if (!node->children.empty())
                        node->expanded = !node->expanded;
                    else if (m_onSelect)
                        m_onSelect(node->label);
                }
            }
            break;
        }

        default: break;
    }
}
