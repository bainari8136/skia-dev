#include <Windows.h>
#include "skChartView.h"
#include "skTheme.h"
#include "skTypeface.h"
#include <include/core/SkPaint.h>
#include <include/core/SkFont.h>
#include <include/core/SkRRect.h>
#include <algorithm>
#include <cstdio>
#include <cstring>

static sk_sp<SkTypeface>& cvTf() {
    static sk_sp<SkTypeface> tf = skGetSystemTypeface();
    return tf;
}

skChartView::skChartView(int sx, int sy, int sw, int sh)
    : skWidget(sx, sy, sw, sh) {}

void skChartView::addBar(const std::string& label, float value) {
    m_bars.push_back({label, value});
}

float skChartView::maxVal() const {
    float mx = 0.f;
    for (auto& b : m_bars) mx = std::max(mx, b.value);
    return mx > 0.f ? mx : 1.f;
}

int skChartView::barAt(int px, int py) const {
    if (m_bars.empty()) return -1;
    float ox = (float)x + kPadL;
    float oy = (float)y + kPadT;
    float cw = chartW(), ch = chartH();
    if (px < (int)ox || px > (int)(ox+cw) || py < (int)oy || py > (int)(oy+ch)) return -1;
    float bw = cw / (float)m_bars.size();
    int idx = (int)((float)(px - (int)ox) / bw);
    return (idx >= 0 && idx < (int)m_bars.size()) ? idx : -1;
}

void skChartView::Paint(SkCanvas* canvas) {
    const auto& th = skGetTheme();
    SkFont font(cvTf(), 10.f);  font.setEdging(SkFont::Edging::kAntiAlias);
    SkFont small(cvTf(),  9.f); small.setEdging(SkFont::Edging::kAntiAlias);

    float fx = (float)x, fy = (float)y, fw = (float)w, fh = (float)h;
    float ox = fx + kPadL, oy = fy + kPadT, cw = chartW(), ch = chartH();

    // Background + border
    SkRRect bg; bg.setRectXY(SkRect::MakeXYWH(fx,fy,fw,fh), 6.f, 6.f);
    SkPaint bgP; bgP.setAntiAlias(true); bgP.setColor(th.panelBg);
    canvas->drawRRect(bg, bgP);
    SkPaint brdP; brdP.setAntiAlias(true);
    brdP.setStyle(SkPaint::kStroke_Style); brdP.setStrokeWidth(1.f);
    brdP.setColor(th.panelBorder);
    canvas->drawRRect(bg, brdP);

    // Title
    if (!m_title.empty()) {
        SkPaint tp; tp.setAntiAlias(true); tp.setColor(th.textPrimary);
        canvas->drawString(m_title.c_str(), ox, fy + 16.f, font, tp);
    }

    // Axes
    SkPaint axP; axP.setAntiAlias(true);
    axP.setStyle(SkPaint::kStroke_Style); axP.setStrokeWidth(1.f);
    axP.setColor(th.panelBorder);
    canvas->drawLine(ox, oy, ox, oy+ch, axP);
    canvas->drawLine(ox, oy+ch, ox+cw, oy+ch, axP);

    // Y-axis grid lines + labels
    const int kTicks = 4;
    float mx = maxVal();
    for (int i = 0; i <= kTicks; ++i) {
        float ty = oy + ch - (float)i / (float)kTicks * ch;
        SkPaint gP; gP.setAntiAlias(true);
        gP.setStyle(SkPaint::kStroke_Style); gP.setStrokeWidth(1.f);
        gP.setColor(SkColorSetARGB(40, 128, 128, 128));
        canvas->drawLine(ox, ty, ox+cw, ty, gP);
        char tv[12]; snprintf(tv, sizeof(tv), "%.0f", mx * (float)i / (float)kTicks);
        SkRect tb; small.measureText(tv, strlen(tv), SkTextEncoding::kUTF8, &tb);
        SkPaint tvP; tvP.setAntiAlias(true); tvP.setColor(th.textSecondary);
        canvas->drawString(tv, ox - 4.f - tb.width() - tb.left(),
                           ty - tb.height()/2.f - tb.top(), small, tvP);
    }

    if (m_bars.empty()) return;

    float bw = cw / (float)m_bars.size();
    float barPad = std::max(2.f, bw * 0.18f);

    for (int i = 0; i < (int)m_bars.size(); ++i) {
        float bx  = ox + (float)i * bw;
        float bh  = (m_bars[i].value / mx) * ch;
        float by  = oy + ch - bh;
        bool  hov = (i == m_hoverIdx);

        // Bar
        SkRRect brr; brr.setRectXY(
            SkRect::MakeXYWH(bx+barPad, by, bw-barPad*2.f, bh), 3.f, 3.f);
        SkPaint bp; bp.setAntiAlias(true);
        bp.setColor(hov ? th.accentHover : th.accent);
        canvas->drawRRect(brr, bp);

        // Value label above bar
        char val[12]; snprintf(val, sizeof(val), "%.0f", m_bars[i].value);
        SkRect vb; small.measureText(val, strlen(val), SkTextEncoding::kUTF8, &vb);
        SkPaint vP; vP.setAntiAlias(true); vP.setColor(th.textSecondary);
        canvas->drawString(val,
            bx+bw/2.f - vb.width()/2.f - vb.left(),
            by - 2.f - vb.bottom(), small, vP);

        // X label below axis
        SkRect lb; small.measureText(m_bars[i].label.c_str(),
            m_bars[i].label.size(), SkTextEncoding::kUTF8, &lb);
        SkPaint lP; lP.setAntiAlias(true);
        lP.setColor(hov ? th.textPrimary : th.textSecondary);
        canvas->drawString(m_bars[i].label.c_str(),
            bx+bw/2.f - lb.width()/2.f - lb.left(),
            oy+ch + 16.f - lb.top(), small, lP);
    }
}

void skChartView::OnEvent(const skEvent& ev) {
    if (ev.type == skEventType::MouseMove)
        m_hoverIdx = barAt(ev.x, ev.y);
}
