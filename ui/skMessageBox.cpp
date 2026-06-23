#include "skMessageBox.h"
#include "skTheme.h"
#include "skTypeface.h"
#include <include/core/SkPaint.h>
#include <include/core/SkRRect.h>
#include <include/core/SkFont.h>
#include <include/core/SkPath.h>
#include <Windows.h>

skMessageBox::skMessageBox(int winW, int winH)
    : skWidget(0, 0, winW, winH) {
    setVisible(false);
}

void skMessageBox::show(const std::string& title, const std::string& message,
                        skMessageType type, skMessageButtons buttons) {
    m_title        = title;
    m_message      = message;
    m_type         = type;
    m_buttons      = buttons;
    m_confirmHover = false;
    m_cancelHover  = false;
    setVisible(true);
}

SkColor skMessageBox::typeColor() const {
    switch (m_type) {
        case skMessageType::Warning: return SkColorSetRGB(0xE3, 0x7B, 0x20); // M3 amber/tertiary
        case skMessageType::Error:   return SkColorSetRGB(0xB3, 0x26, 0x1E); // M3 error
        case skMessageType::Success: return SkColorSetRGB(0x38, 0x6A, 0x20); // M3 green
        default:                     return skGetTheme().accent;               // primary
    }
}

void skMessageBox::drawIcon(SkCanvas* canvas, float cx, float cy) const {
    static sk_sp<SkTypeface> tf = skGetSystemTypeface();
    const SkColor col = typeColor();
    const uint8_t cr = SkColorGetR(col), cg = SkColorGetG(col), cb = SkColorGetB(col);

    // M3 icon container: tinted circle at 15% opacity
    SkPaint bg; bg.setAntiAlias(true);
    bg.setColor(SkColorSetARGB(38, cr, cg, cb));
    canvas->drawCircle(cx, cy, kIconR, bg);

    if (m_type == skMessageType::Success) {
        // Checkmark path — more legible than a Unicode glyph
        SkPaint cp; cp.setAntiAlias(true);
        cp.setStyle(SkPaint::kStroke_Style);
        cp.setStrokeWidth(2.2f);
        cp.setStrokeCap(SkPaint::kRound_Cap);
        cp.setStrokeJoin(SkPaint::kRound_Join);
        cp.setColor(col);
        SkPath path;
        path.moveTo(cx - 7.f, cy + 1.f);
        path.lineTo(cx - 1.5f, cy + 7.f);
        path.lineTo(cx + 8.f,  cy - 6.f);
        canvas->drawPath(path, cp);
    } else if (m_type == skMessageType::Warning) {
        // Exclamation "!" — warning and info use circle + symbol
        const char* sym = "!";
        SkFont f(tf, 17.f);
        f.setEdging(SkFont::Edging::kAntiAlias);
        SkRect b;
        f.measureText(sym, strlen(sym), SkTextEncoding::kUTF8, &b);
        SkPaint tp; tp.setAntiAlias(true); tp.setColor(col);
        canvas->drawString(sym,
            cx - b.width() * 0.5f - b.left(),
            cy - b.height() * 0.5f - b.top(),
            f, tp);
    } else if (m_type == skMessageType::Error) {
        // "×" for error (indicates failure, distinct from the close button action)
        const char* sym = "\xc3\x97"; // UTF-8 ×
        SkFont f(tf, 17.f);
        f.setEdging(SkFont::Edging::kAntiAlias);
        SkRect b;
        f.measureText(sym, strlen(sym), SkTextEncoding::kUTF8, &b);
        SkPaint tp; tp.setAntiAlias(true); tp.setColor(col);
        canvas->drawString(sym,
            cx - b.width() * 0.5f - b.left(),
            cy - b.height() * 0.5f - b.top(),
            f, tp);
    } else {
        // Info: lowercase "i" with a dot above (italic look)
        const char* sym = "i";
        SkFont f(tf, 17.f);
        f.setEdging(SkFont::Edging::kAntiAlias);
        SkRect b;
        f.measureText(sym, strlen(sym), SkTextEncoding::kUTF8, &b);
        SkPaint tp; tp.setAntiAlias(true); tp.setColor(col);
        canvas->drawString(sym,
            cx - b.width() * 0.5f - b.left(),
            cy - b.height() * 0.5f - b.top(),
            f, tp);
    }
}

const char* skMessageBox::confirmLabel() const {
    return (m_buttons == skMessageButtons::YesNo) ? "Yes" : "OK";
}

const char* skMessageBox::cancelLabel() const {
    return (m_buttons == skMessageButtons::YesNo) ? "No" : "Cancel";
}

bool skMessageBox::inConfirm(int px, int py) const {
    return px >= (int)confirmBtnX() && px < (int)(confirmBtnX() + kBtnW) &&
           py >= (int)confirmBtnY() && py < (int)(confirmBtnY() + kBtnH);
}

bool skMessageBox::inCancel(int px, int py) const {
    return hasCancel() &&
           px >= (int)cancelBtnX() && px < (int)(cancelBtnX() + kBtnW) &&
           py >= (int)cancelBtnY() && py < (int)(cancelBtnY() + kBtnH);
}

void skMessageBox::Paint(SkCanvas* canvas) {
    const auto& th = skGetTheme();
    static sk_sp<SkTypeface> tf = skGetSystemTypeface();

    // M3 scrim: #000000 at ~50% — blocks interaction with content behind
    SkPaint scrim; scrim.setColor(SkColorSetARGB(128, 0, 0, 0));
    canvas->drawRect(SkRect::MakeXYWH((float)x, (float)y, (float)w, (float)h), scrim);

    const float dx = dlgX(), dy_ = dlgY();
    const SkRect dlgRect = SkRect::MakeXYWH(dx, dy_, (float)kDlgW, (float)kDlgH);
    SkRRect rr; rr.setRectXY(dlgRect, kRadius, kRadius);

    // M3 elevation level-3 shadow: 3 stacked offset rects (no blur available cheaply)
    struct ShadowLayer { float yo; uint8_t alpha; };
    for (auto [yo, alpha] : { ShadowLayer{8.f, 24}, {4.f, 16}, {2.f, 10} }) {
        SkRRect sr;
        sr.setRectXY(SkRect::MakeXYWH(dx, dy_ + yo, (float)kDlgW, (float)kDlgH),
                     kRadius, kRadius);
        SkPaint sh; sh.setAntiAlias(true); sh.setColor(SkColorSetARGB(alpha, 0, 0, 0));
        canvas->drawRRect(sr, sh);
    }

    // Dialog surface — M3 surface-container-high (white in light theme)
    SkPaint bg; bg.setAntiAlias(true); bg.setColor(th.panelBg);
    canvas->drawRRect(rr, bg);

    // Layout (top → bottom, all horizontally centered — M3 icon-dialog layout)
    // Icon: center of circle = top-pad + icon-radius
    const float iconCX = dx + (float)kDlgW * 0.5f;
    const float iconCY = dy_ + kPad + kIconR;
    // Headline baseline origin: 20dp below icon bottom
    const float headY  = iconCY + kIconR + 20.f;
    // Body text: 34dp below headline origin
    const float bodyY  = headY + 34.f;

    // Icon
    drawIcon(canvas, iconCX, iconCY);

    // Headline — M3 headlineSmall (22sp), centered, on-surface
    SkFont headFont(tf, 22.f);
    headFont.setEdging(SkFont::Edging::kAntiAlias);
    SkRect hb;
    headFont.measureText(m_title.c_str(), m_title.size(), SkTextEncoding::kUTF8, &hb);
    SkPaint hp; hp.setAntiAlias(true); hp.setColor(th.textPrimary);
    canvas->drawString(m_title.c_str(),
        iconCX - hb.width() * 0.5f - hb.left(),
        headY  - hb.top(),
        headFont, hp);

    // Body text — M3 bodyMedium (13sp), centered, on-surface-variant
    SkFont bodyFont(tf, 13.f);
    bodyFont.setEdging(SkFont::Edging::kAntiAlias);
    SkPaint bp2; bp2.setAntiAlias(true); bp2.setColor(th.textSecondary);

    auto drawCenteredLine = [&](const std::string& line, float ly) {
        if (line.empty()) return;
        SkRect lb;
        bodyFont.measureText(line.c_str(), line.size(), SkTextEncoding::kUTF8, &lb);
        canvas->drawString(line.c_str(),
            iconCX - lb.width() * 0.5f - lb.left(),
            ly - lb.top(),
            bodyFont, bp2);
    };

    auto nl = m_message.find('\n');
    if (nl != std::string::npos) {
        drawCenteredLine(m_message.substr(0, nl),  bodyY);
        drawCenteredLine(m_message.substr(nl + 1), bodyY + 20.f);
    } else {
        drawCenteredLine(m_message, bodyY);
    }

    // Buttons — right-aligned row at dialog bottom
    const SkColor col = typeColor();

    // Cancel — M3 "text" button: no background, label in primary/type color
    if (hasCancel()) {
        const float cx2 = cancelBtnX(), cy2 = cancelBtnY();
        if (m_cancelHover) {
            // M3 state layer: 8% primary color on hover
            SkRRect cbr; cbr.setRectXY(SkRect::MakeXYWH(cx2, cy2, kBtnW, kBtnH), kBtnR, kBtnR);
            SkPaint hl; hl.setAntiAlias(true);
            hl.setColor(SkColorSetARGB(20, SkColorGetR(col), SkColorGetG(col), SkColorGetB(col)));
            canvas->drawRRect(cbr, hl);
        }
        const char* lbl = cancelLabel();
        SkFont cf(tf, 14.f); cf.setEdging(SkFont::Edging::kAntiAlias);
        SkRect ctb;
        cf.measureText(lbl, strlen(lbl), SkTextEncoding::kUTF8, &ctb);
        SkPaint ctp; ctp.setAntiAlias(true); ctp.setColor(col);
        canvas->drawString(lbl,
            cx2 + (kBtnW - ctb.width()) * 0.5f - ctb.left(),
            cy2 + (kBtnH - ctb.height()) * 0.5f - ctb.top(),
            cf, ctp);
    }

    // Confirm — M3 "filled" button: solid type-color background, white label
    {
        const float bx = confirmBtnX(), by2 = confirmBtnY();
        SkRRect brr; brr.setRectXY(SkRect::MakeXYWH(bx, by2, kBtnW, kBtnH), kBtnR, kBtnR);
        // M3 hover: lighten fill by 12%
        SkColor fill = m_confirmHover ? skLerpColor(col, SK_ColorWHITE, 0.12f) : col;
        SkPaint fp; fp.setAntiAlias(true); fp.setColor(fill);
        canvas->drawRRect(brr, fp);

        const char* lbl = confirmLabel();
        SkFont f(tf, 14.f); f.setEdging(SkFont::Edging::kAntiAlias);
        SkRect tb;
        f.measureText(lbl, strlen(lbl), SkTextEncoding::kUTF8, &tb);
        SkPaint tp; tp.setAntiAlias(true); tp.setColor(SK_ColorWHITE);
        canvas->drawString(lbl,
            bx + (kBtnW - tb.width()) * 0.5f - tb.left(),
            by2 + (kBtnH - tb.height()) * 0.5f - tb.top(),
            f, tp);
    }
}

bool skMessageBox::handleEvent(const skEvent& ev) {
    if (!visible()) return false;

    if (ev.type == skEventType::KeyDown) {
        if (ev.button == VK_ESCAPE) {
            setVisible(false);
            if (m_onCancel)  m_onCancel();
            else if (m_onConfirm) m_onConfirm(); // OK-only dialog: Escape still closes
            return true;
        }
        if (ev.button == VK_RETURN) {
            setVisible(false);
            if (m_onConfirm) m_onConfirm();
            return true;
        }
    }

    if (ev.type == skEventType::MouseMove) {
        m_confirmHover = inConfirm(ev.x, ev.y);
        m_cancelHover  = inCancel(ev.x, ev.y);
        return true; // consume all mouse moves while dialog is open
    }

    if (ev.type == skEventType::MouseDown) {
        if (inConfirm(ev.x, ev.y)) {
            setVisible(false);
            if (m_onConfirm) m_onConfirm();
        } else if (inCancel(ev.x, ev.y)) {
            setVisible(false);
            if (m_onCancel) m_onCancel();
        }
        return true; // always consume while visible
    }

    if (ev.type == skEventType::MouseUp) return true;

    return false;
}
