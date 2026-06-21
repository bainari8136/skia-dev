#include "skExpander.h"
#include "skTypeface.h"
#include "skTheme.h"
#include <include/core/SkPaint.h>
#include <include/core/SkFont.h>
#include <include/core/SkPath.h>
#include <algorithm>

skExpander::skExpander(int ex, int ey, int ew, int totalH, int headerH, std::string title)
    : skWidget(ex, ey, ew, totalH), m_headerH(headerH), m_title(std::move(title)) {}

void skExpander::addChild(std::shared_ptr<skWidget> child) {
    m_children.push_back(std::move(child));
}

void skExpander::setExpanded(bool v) {
    m_expanded = v;
}

// ---------------------------------------------------------------------------
// Tick — animate open/close
// ---------------------------------------------------------------------------

void skExpander::onTick() {
    float target = m_expanded ? 1.f : 0.f;
    float step   = 0.12f;
    if (m_openAnim < target) m_openAnim = std::min(1.f, m_openAnim + step);
    else                     m_openAnim = std::max(0.f, m_openAnim - step);
}

// ---------------------------------------------------------------------------
// Paint
// ---------------------------------------------------------------------------

void skExpander::Paint(SkCanvas* canvas) {
    const auto& th = skGetTheme();

    float fx = (float)x;
    float fy = (float)y;
    float fw = (float)w;
    float fh = (float)m_headerH;

    // Header background
    SkPaint hbg;
    hbg.setAntiAlias(true);
    hbg.setColor(m_hovered ? SkColorSetARGB(18, 128, 128, 128) : th.panelBg);
    canvas->drawRect(SkRect::MakeXYWH(fx, fy, fw, fh), hbg);

    // Accent left stripe on header
    SkPaint stripe;
    stripe.setColor(th.accent);
    canvas->drawRect(SkRect::MakeXYWH(fx, fy, 3.f, fh), stripe);

    // Chevron icon (right-pointing = collapsed, down-pointing = open)
    {
        float cx  = fx + 18.f;
        float cy  = fy + fh / 2.f;
        float sz  = 5.f;
        // Lerp between right-arrow (collapsed) and down-arrow (open)
        SkPath chevron;
        // Right-arrow points: top-right, bottom-right, left
        // Down-arrow points:  left, right, bottom
        // Animate by rotating the chevron 90° as m_openAnim goes 0→1
        float angle = m_openAnim * 3.14159f / 2.f; // 0 = right, π/2 = down
        float cosA  = cosf(angle);
        float sinA  = sinf(angle);
        auto rot    = [&](float lx, float ly) -> std::pair<float, float> {
            return { cx + lx * cosA - ly * sinA,
                     cy + lx * sinA + ly * cosA };
        };
        auto [ax, ay] = rot(-sz,  sz);
        auto [bx, by] = rot( sz,  0.f);
        auto [ccx, ccy] = rot(-sz, -sz);
        chevron.moveTo(ax, ay);
        chevron.lineTo(bx, by);
        chevron.lineTo(ccx, ccy);
        SkPaint cp;
        cp.setAntiAlias(true);
        cp.setStyle(SkPaint::kStroke_Style);
        cp.setStrokeWidth(1.8f);
        cp.setStrokeCap(SkPaint::kRound_Cap);
        cp.setStrokeJoin(SkPaint::kRound_Join);
        cp.setColor(th.textSecondary);
        canvas->drawPath(chevron, cp);
    }

    // Title text
    {
        static sk_sp<SkTypeface> s_tf = skGetSystemTypeface();
        SkFont font(s_tf, 13.f);
        font.setEdging(SkFont::Edging::kAntiAlias);
        SkRect tb;
        font.measureText(m_title.c_str(), m_title.size(), SkTextEncoding::kUTF8, &tb);
        SkPaint tp;
        tp.setAntiAlias(true);
        tp.setColor(th.textPrimary);
        canvas->drawString(m_title.c_str(),
            fx + 34.f - tb.left(),
            fy + (fh - tb.height()) / 2.f - tb.top(),
            font, tp);
    }

    // Header bottom border
    SkPaint div;
    div.setColor(th.panelBorder);
    canvas->drawLine(fx, fy + fh, fx + fw, fy + fh, div);

    // Content area — clip to animated height
    float clipH = (float)contentH() * m_openAnim;
    if (clipH < 0.5f) return;

    canvas->save();
    canvas->clipRect(SkRect::MakeLTRB(fx, fy + fh, fx + fw, fy + fh + clipH));
    canvas->translate(fx, fy + (float)m_headerH);

    for (auto& c : m_children)
        if (c->visible()) c->Paint(canvas);

    canvas->restore();
}

// ---------------------------------------------------------------------------
// Events
// ---------------------------------------------------------------------------

void skExpander::OnEvent(const skEvent& ev) {
    switch (ev.type) {
        case skEventType::MouseMove:
            m_hovered = contains(ev.x, ev.y) && (ev.y < y + m_headerH);
            // Forward mouse events to children in local coords
            if (m_openAnim > 0.f && ev.y >= y + m_headerH) {
                skEvent local = ev;
                local.x = ev.x - x;
                local.y = ev.y - y - m_headerH;
                for (auto& c : m_children) if (c->visible()) c->OnEvent(local);
            }
            break;

        case skEventType::MouseDown:
            if (contains(ev.x, ev.y) && ev.y < y + m_headerH) {
                m_expanded = !m_expanded;
            } else if (m_openAnim > 0.f && ev.y >= y + m_headerH) {
                skEvent local = ev;
                local.x = ev.x - x;
                local.y = ev.y - y - m_headerH;
                for (auto& c : m_children) if (c->visible()) c->OnEvent(local);
            }
            break;

        case skEventType::MouseUp:
            if (m_openAnim > 0.f && ev.y >= y + m_headerH) {
                skEvent local = ev;
                local.x = ev.x - x;
                local.y = ev.y - y - m_headerH;
                for (auto& c : m_children) if (c->visible()) c->OnEvent(local);
            }
            break;

        default: break;
    }
}
