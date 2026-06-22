#include "skPopover.h"
#include "skTheme.h"
#include "skTypeface.h"
#include <include/core/SkPaint.h>
#include <include/core/SkFont.h>
#include <include/core/SkRRect.h>
#include <include/core/SkPath.h>
#include <Windows.h>

static sk_sp<SkTypeface>& popTf() {
    static sk_sp<SkTypeface> tf = skGetSystemTypeface();
    return tf;
}

skPopover::skPopover(int contentW, int contentH)
    : skWidget(0, 0, contentW, contentH) {
    setVisible(false);
}

void skPopover::openAt(int ax, int ay) {
    // Position centered on ax, bottom-aligned to ay (arrow tip touches anchor)
    x = ax - w / 2;
    y = ay - h - (int)kArrow - 2;
    setVisible(true);
}

void skPopover::Paint(SkCanvas* canvas) {
    const auto& th = skGetTheme();

    // Drop shadow
    SkRRect sh; sh.setRectXY(
        SkRect::MakeXYWH((float)x + 2, (float)y + 5, (float)w, (float)h), kRadius, kRadius);
    SkPaint shp; shp.setAntiAlias(true); shp.setColor(SkColorSetARGB(28, 0, 0, 0));
    canvas->drawRRect(sh, shp);

    // Body
    SkRRect rr; rr.setRectXY(
        SkRect::MakeXYWH((float)x, (float)y, (float)w, (float)h), kRadius, kRadius);
    SkPaint bg; bg.setAntiAlias(true); bg.setColor(th.panelBg);
    canvas->drawRRect(rr, bg);
    SkPaint brd; brd.setAntiAlias(true);
    brd.setStyle(SkPaint::kStroke_Style); brd.setStrokeWidth(0.75f);
    brd.setColor(th.panelBorder);
    canvas->drawRRect(rr, brd);

    // Downward arrow at bottom-center
    float arrowCX = (float)x + (float)w * 0.5f;
    float arrowTY = (float)(y + h);
    SkPath arrow;
    arrow.moveTo(arrowCX - kArrow, arrowTY - 1.f);
    arrow.lineTo(arrowCX + kArrow, arrowTY - 1.f);
    arrow.lineTo(arrowCX,          arrowTY + kArrow);
    arrow.close();
    SkPaint ap; ap.setAntiAlias(true); ap.setColor(th.panelBg);
    canvas->drawPath(arrow, ap);
    ap.setStyle(SkPaint::kStroke_Style); ap.setStrokeWidth(0.75f); ap.setColor(th.panelBorder);
    canvas->drawPath(arrow, ap);

    float curY = (float)y + kPadY;

    // Title
    if (!m_title.empty()) {
        SkFont tf(popTf(), 13.f); tf.setEdging(SkFont::Edging::kAntiAlias);
        SkRect tb;
        tf.measureText(m_title.c_str(), m_title.size(), SkTextEncoding::kUTF8, &tb);
        SkPaint tp; tp.setAntiAlias(true); tp.setColor(skGetTheme().textPrimary);
        canvas->drawString(m_title.c_str(), (float)x + kPadX - tb.left(), curY - tb.top(), tf, tp);
        curY += 20.f;
        SkPaint div; div.setColor(th.panelBorder);
        canvas->drawLine((float)x + 8.f, curY - 4.f, (float)(x + w) - 8.f, curY - 4.f, div);
    }

    // Lines
    SkFont lf(popTf(), 12.f); lf.setEdging(SkFont::Edging::kAntiAlias);
    SkPaint ltp; ltp.setAntiAlias(true); ltp.setColor(th.textSecondary);
    for (const auto& line : m_lines) {
        SkRect lb;
        lf.measureText(line.c_str(), line.size(), SkTextEncoding::kUTF8, &lb);
        canvas->drawString(line.c_str(), (float)x + kPadX - lb.left(), curY - lb.top(), lf, ltp);
        curY += kLineH;
    }
}

bool skPopover::handleEvent(const skEvent& ev) {
    if (!visible()) return false;
    if (ev.type == skEventType::KeyDown && ev.button == VK_ESCAPE) {
        setVisible(false); return true;
    }
    if (ev.type == skEventType::MouseDown) {
        bool inside = (ev.x >= x && ev.x < x + w &&
                       ev.y >= y && ev.y < y + h + (int)kArrow + 2);
        if (!inside) setVisible(false);
        return inside;
    }
    if (ev.type == skEventType::MouseMove) return visible();
    return false;
}
