#include "skExpander.h"
#include "skTypeface.h"
#include "skTheme.h"
#include <include/core/SkPaint.h>
#include <include/core/SkFont.h>
#include <include/core/SkPath.h>
#include <include/core/SkRRect.h>
#include <algorithm>
#include <cmath>

static SkColor withAlpha(SkColor c, uint8_t a) {
    return SkColorSetARGB(a, SkColorGetR(c), SkColorGetG(c), SkColorGetB(c));
}

skExpander::skExpander(int ex, int ey, int ew, int totalH, int headerH, std::string title)
    : skWidget(ex, ey, ew, totalH), m_headerH(headerH), m_title(std::move(title)) {}

void skExpander::addChild(std::shared_ptr<skWidget> child) {
    m_children.push_back(std::move(child));
}

void skExpander::setExpanded(bool v) {
    m_expanded = v;
    m_openAnim = v ? 1.f : 0.f;
}

// ── Animation ─────────────────────────────────────────────────────────────────
// Exponential decay toward target — approximates M3's decelerated easing (~300ms at 60fps).
void skExpander::onTick() {
    float target = m_expanded ? 1.f : 0.f;
    if (m_openAnim == target) return;
    m_openAnim += (target - m_openAnim) * 0.20f;
    if (std::abs(m_openAnim - target) < 0.005f) m_openAnim = target;
}

// ── Paint ─────────────────────────────────────────────────────────────────────
void skExpander::Paint(SkCanvas* canvas) {
    const auto& th = skGetTheme();

    float fx   = (float)x;
    float fy   = (float)y;
    float fw   = (float)w;
    float fhdr = (float)m_headerH;
    float clipH = (float)contentH() * m_openAnim;
    float animH = fhdr + clipH;
    constexpr float kContR = 12.f; // M3 container corner radius

    // ── CONTAINER ─────────────────────────────────────────────────────────────
    SkRect animRect = SkRect::MakeXYWH(fx, fy, fw, animH);
    SkRRect animRR; animRR.setRectXY(animRect, kContR, kContR);

    // Background (surfaceContainerLow)
    SkPaint bgP; bgP.setAntiAlias(true); bgP.setColor(th.panelBg);
    canvas->drawRRect(animRR, bgP);

    // ── HEADER HOVER STATE LAYER (8% onSurface) ────────────────────────────
    if (m_hovered) {
        SkRect hdrRect = SkRect::MakeXYWH(fx, fy, fw, fhdr);
        SkRRect hoverRR;
        if (clipH < 1.f) {
            // Collapsed: all four corners rounded
            hoverRR.setRectXY(hdrRect, kContR, kContR);
        } else {
            // Expanded: only top corners rounded (bottom is flush with content)
            const SkVector radii[4] = {
                {kContR, kContR}, {kContR, kContR}, // upper-left, upper-right
                {0.f, 0.f},       {0.f, 0.f}         // lower-right, lower-left
            };
            hoverRR.setRectRadii(hdrRect, radii);
        }
        SkPaint hp; hp.setAntiAlias(true); hp.setColor(withAlpha(th.textPrimary, 20));
        canvas->drawRRect(hoverRR, hp);
    }

    // ── TITLE — bodyLarge (16sp), 16dp left padding ────────────────────────
    {
        static sk_sp<SkTypeface> s_tf = skGetSystemTypeface();
        SkFont font(s_tf, 16.f);
        font.setEdging(SkFont::Edging::kAntiAlias);
        SkRect tb;
        font.measureText(m_title.c_str(), m_title.size(), SkTextEncoding::kUTF8, &tb);
        SkPaint tp; tp.setAntiAlias(true); tp.setColor(th.textPrimary);
        canvas->drawString(m_title.c_str(),
            fx + 16.f - tb.left(),
            fy + fhdr * 0.5f - tb.height() * 0.5f - tb.top(),
            font, tp);
    }

    // ── TRAILING CHEVRON ───────────────────────────────────────────────────
    // Starts pointing down (∨ = collapsed) and rotates 180° to point up (∧ = expanded).
    {
        float cx    = fx + fw - 20.f;
        float cy    = fy + fhdr * 0.5f;
        float sz    = 5.5f;
        float angle = m_openAnim * 3.14159265f; // 0 = ∨, π = ∧
        float cosA  = cosf(angle);
        float sinA  = sinf(angle);
        auto rot = [&](float lx, float ly) -> SkPoint {
            return { cx + lx * cosA - ly * sinA,
                     cy + lx * sinA + ly * cosA };
        };
        SkPath chevron;
        chevron.moveTo(rot(-sz * 1.1f, -sz * 0.5f));
        chevron.lineTo(rot( 0.f,        sz * 0.5f));
        chevron.lineTo(rot( sz * 1.1f, -sz * 0.5f));
        SkPaint cp;
        cp.setAntiAlias(true);
        cp.setStyle(SkPaint::kStroke_Style);
        cp.setStrokeWidth(2.f);
        cp.setStrokeCap(SkPaint::kRound_Cap);
        cp.setStrokeJoin(SkPaint::kRound_Join);
        cp.setColor(withAlpha(th.textPrimary, 160));
        canvas->drawPath(chevron, cp);
    }

    // ── HEADER / CONTENT DIVIDER ──────────────────────────────────────────
    // Fades in over the first 20px of opening so it doesn't pop.
    if (clipH > 0.5f) {
        float divAlpha = 28.f * std::min(1.f, clipH / 20.f);
        SkPaint div; div.setAntiAlias(true);
        div.setStyle(SkPaint::kStroke_Style); div.setStrokeWidth(1.f);
        div.setColor(withAlpha(th.textPrimary, (uint8_t)divAlpha));
        canvas->drawLine(fx + 16.f, fy + fhdr, fx + fw - 16.f, fy + fhdr, div);
    }

    // ── CONTAINER BORDER (onSurface@12%) ─────────────────────────────────
    {
        SkPaint brd; brd.setAntiAlias(true);
        brd.setStyle(SkPaint::kStroke_Style); brd.setStrokeWidth(1.f);
        brd.setColor(withAlpha(th.textPrimary, 30));
        canvas->drawRRect(animRR, brd);
    }

    // ── CONTENT AREA ─────────────────────────────────────────────────────
    if (clipH < 0.5f) return;

    canvas->save();
    canvas->clipRRect(animRR, SkClipOp::kIntersect, true);
    canvas->translate(fx, fy + fhdr);

    for (auto& c : m_children)
        if (c->visible()) c->Paint(canvas);

    canvas->restore();
}

// ── Events ────────────────────────────────────────────────────────────────────

void skExpander::OnEvent(const skEvent& ev) {
    int visibleBottom = y + m_headerH + (int)((float)contentH() * m_openAnim);

    auto forwardToChildren = [&]() {
        skEvent local = ev;
        local.x = ev.x - x;
        local.y = ev.y - y - m_headerH;
        for (auto& c : m_children) if (c->visible()) c->OnEvent(local);
    };

    bool inContent = m_openAnim > 0.f
                  && ev.y >= y + m_headerH
                  && ev.y <  visibleBottom;

    switch (ev.type) {
        case skEventType::MouseMove:
            m_hovered = contains(ev.x, ev.y) && (ev.y < y + m_headerH);
            if (inContent) forwardToChildren();
            break;

        case skEventType::MouseDown:
            if (contains(ev.x, ev.y) && ev.y < y + m_headerH)
                m_expanded = !m_expanded;
            else if (inContent)
                forwardToChildren();
            break;

        case skEventType::MouseUp:
            if (inContent) forwardToChildren();
            break;

        default: break;
    }
}
