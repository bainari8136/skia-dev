#include <Windows.h>
#include "skConsoleView.h"
#include "skTheme.h"
#include "skTypeface.h"
#include <include/core/SkPaint.h>
#include <include/core/SkFont.h>
#include <include/core/SkRRect.h>
#include <algorithm>

static sk_sp<SkTypeface>& conTf() {
    static sk_sp<SkTypeface> tf = skGetSystemTypeface();
    return tf;
}

skConsoleView::skConsoleView(int sx, int sy, int sw, int sh)
    : skWidget(sx, sy, sw, sh) {}

void skConsoleView::clear() { m_lines.clear(); m_scrollOff = 0; }

void skConsoleView::appendLine(const std::string& text, SkColor color) {
    m_lines.push_back({text, color});
    if (m_scrollOff == 0) {
        // stay at bottom — nothing to adjust
    } else {
        // keep user's view position
        ++m_scrollOff;
    }
}

void skConsoleView::Paint(SkCanvas* canvas) {
    const auto& th = skGetTheme();

    float fx = (float)x, fy = (float)y, fw = (float)w, fh = (float)h;

    // Dark background
    SkRRect bg; bg.setRectXY(SkRect::MakeXYWH(fx,fy,fw,fh), 4.f, 4.f);
    SkPaint bgP; bgP.setAntiAlias(true);
    bgP.setColor(SkColorSetRGB(20, 22, 28));
    canvas->drawRRect(bg, bgP);

    SkPaint brdP; brdP.setAntiAlias(true);
    brdP.setStyle(SkPaint::kStroke_Style); brdP.setStrokeWidth(1.f);
    brdP.setColor(th.panelBorder);
    canvas->drawRRect(bg, brdP);

    SkFont font(conTf(), kFontSz); font.setEdging(SkFont::Edging::kAntiAlias);

    canvas->save();
    canvas->clipRect(SkRect::MakeXYWH(fx, fy, fw, fh));

    int vis   = visibleLines();
    int total = (int)m_lines.size();
    int start = std::max(0, total - vis - m_scrollOff);
    int end   = std::min(total, start + vis);

    for (int i = start; i < end; ++i) {
        float lineY = fy + (float)(i - start) * kLineH + kLineH - 3.f;
        SkPaint lp; lp.setAntiAlias(true); lp.setColor(m_lines[i].color);
        canvas->drawString(m_lines[i].text.c_str(), fx + kPadX, lineY, font, lp);
    }

    canvas->restore();

    // Scrollbar
    int ms = maxScroll();
    if (ms > 0) {
        float sbX  = fx + fw - kSbW;
        SkPaint trk; trk.setColor(SkColorSetRGB(35, 37, 45));
        canvas->drawRect(SkRect::MakeXYWH(sbX, fy, kSbW, fh), trk);

        float ratio   = (float)vis / (float)(vis + ms);
        float thumbH  = std::max(20.f, fh * ratio);
        float scrollT = (float)(ms - m_scrollOff) / (float)ms;
        float thumbY  = fy + scrollT * (fh - thumbH);
        SkRRect tRR; tRR.setRectXY(SkRect::MakeXYWH(sbX+1.f,thumbY+1.f,kSbW-2.f,thumbH-2.f),3.f,3.f);
        SkPaint tP; tP.setAntiAlias(true); tP.setColor(SkColorSetRGB(80,84,96));
        canvas->drawRRect(tRR, tP);
    }
}

void skConsoleView::OnEvent(const skEvent& ev) {
    // Drag move/up — handled before the contains guard so drag tracks cursor outside widget
    if (ev.type == skEventType::MouseMove && m_sbDrag) {
        int ms = maxScroll();
        if (m_sbDragTrack > 0.f && ms > 0) {
            int delta = ev.y - m_sbDragY;
            // Inverted convention: drag DOWN → scroll toward bottom → m_scrollOff decreases
            int newOff  = m_sbDragOff - (int)((float)delta / m_sbDragTrack * (float)ms);
            m_scrollOff = std::max(0, std::min(ms, newOff));
        }
        return;
    }
    if ((ev.type == skEventType::MouseUp || ev.type == skEventType::MouseCancel) && m_sbDrag) {
        m_sbDrag = false;
        return;
    }

    if (!contains(ev.x, ev.y)) return;

    // Scrollbar strip: begin drag on click
    if (ev.type == skEventType::MouseDown && ev.x >= x + w - (int)kSbW) {
        int ms = maxScroll();
        if (ms > 0) {
            int vis       = visibleLines();
            float fh      = (float)h;
            float tH      = std::max(20.f, fh * (float)vis / (float)(vis + ms));
            m_sbDrag      = true;
            m_sbDragY     = ev.y;
            m_sbDragOff   = m_scrollOff;
            m_sbDragThumb = tH;
            m_sbDragTrack = fh - tH;
        }
        return;
    }

    if (ev.type == skEventType::MouseWheel) {
        int ms = maxScroll();
        if (ev.button > 0) m_scrollOff = std::min(ms, m_scrollOff + 2);
        else               m_scrollOff = std::max(0,  m_scrollOff - 2);
    }
}
