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
                        skMessageType type) {
    m_title   = title;
    m_message = message;
    m_type    = type;
    m_okHover = false;
    setVisible(true);
}

SkColor skMessageBox::typeColor() const {
    switch (m_type) {
        case skMessageType::Warning: return SkColorSetRGB(220, 140,  20);
        case skMessageType::Error:   return SkColorSetRGB(210,  50,  40);
        default:                     return skGetTheme().accent;
    }
}

const char* skMessageBox::typeLabel() const {
    switch (m_type) {
        case skMessageType::Warning: return "!";
        case skMessageType::Error:   return "\xc3\x97"; // UTF-8 ×
        default:                     return "i";
    }
}

void skMessageBox::Paint(SkCanvas* canvas) {
    const auto& th = skGetTheme();

    // Backdrop
    SkPaint bkp; bkp.setColor(SkColorSetARGB(140, 0, 0, 0));
    canvas->drawRect(SkRect::MakeXYWH((float)x, (float)y, (float)w, (float)h), bkp);

    const float dx = (float)dlgX(), dy = (float)dlgY();

    // Dialog shadow (two layers)
    SkRRect s1; s1.setRectXY(SkRect::MakeXYWH(dx + 2, dy + 6, (float)kDlgW, (float)kDlgH), kRadius, kRadius);
    SkPaint sh1; sh1.setAntiAlias(true); sh1.setColor(SkColorSetARGB(28, 0, 0, 0));
    canvas->drawRRect(s1, sh1);

    SkRRect s2; s2.setRectXY(SkRect::MakeXYWH(dx + 1, dy + 3, (float)kDlgW, (float)kDlgH), kRadius, kRadius);
    SkPaint sh2; sh2.setAntiAlias(true); sh2.setColor(SkColorSetARGB(16, 0, 0, 0));
    canvas->drawRRect(s2, sh2);

    // Dialog background
    SkRRect rr; rr.setRectXY(SkRect::MakeXYWH(dx, dy, (float)kDlgW, (float)kDlgH), kRadius, kRadius);
    SkPaint bg; bg.setAntiAlias(true); bg.setColor(th.panelBg);
    canvas->drawRRect(rr, bg);

    // Colored top bar (clipped to top rounded corners)
    canvas->save();
    canvas->clipRRect(rr, SkClipOp::kIntersect, true);
    SkPaint bar; bar.setAntiAlias(true); bar.setColor(typeColor());
    canvas->drawRect(SkRect::MakeXYWH(dx, dy, (float)kDlgW, kBarH), bar);
    canvas->restore();

    // Dialog border
    SkPaint brd; brd.setAntiAlias(true);
    brd.setStyle(SkPaint::kStroke_Style);
    brd.setStrokeWidth(0.75f);
    brd.setColor(th.panelBorder);
    canvas->drawRRect(rr, brd);

    static sk_sp<SkTypeface> tf = skGetSystemTypeface();

    // Icon circle
    const float iconCX = dx + 38.f, iconCY = dy + kBarH + 52.f;
    const float iconR  = 18.f;
    SkPaint iconBg; iconBg.setAntiAlias(true);
    iconBg.setColor(SkColorSetARGB(30, SkColorGetR(typeColor()),
                                       SkColorGetG(typeColor()),
                                       SkColorGetB(typeColor())));
    canvas->drawCircle(iconCX, iconCY, iconR, iconBg);
    SkPaint iconBrd; iconBrd.setAntiAlias(true);
    iconBrd.setStyle(SkPaint::kStroke_Style);
    iconBrd.setStrokeWidth(1.5f);
    iconBrd.setColor(typeColor());
    canvas->drawCircle(iconCX, iconCY, iconR, iconBrd);

    SkFont iconFont(tf, 14.f);
    iconFont.setEdging(SkFont::Edging::kAntiAlias);
    const char* lbl = typeLabel();
    SkRect ib;
    iconFont.measureText(lbl, strlen(lbl), SkTextEncoding::kUTF8, &ib);
    SkPaint ip; ip.setAntiAlias(true); ip.setColor(typeColor());
    canvas->drawString(lbl,
        iconCX - ib.width() * 0.5f - ib.left(),
        iconCY - ib.height() * 0.5f - ib.top(),
        iconFont, ip);

    // Title
    SkFont titleFont(tf, 15.f);
    titleFont.setEdging(SkFont::Edging::kAntiAlias);
    SkRect ttb;
    titleFont.measureText(m_title.c_str(), m_title.size(), SkTextEncoding::kUTF8, &ttb);
    SkPaint ttp; ttp.setAntiAlias(true); ttp.setColor(th.textPrimary);
    canvas->drawString(m_title.c_str(),
        dx + 70.f - ttb.left(),
        dy + kBarH + 30.f - ttb.top(),
        titleFont, ttp);

    // Message (split on first \n for optional second line)
    SkFont msgFont(tf, 12.f);
    msgFont.setEdging(SkFont::Edging::kAntiAlias);
    SkPaint mtp; mtp.setAntiAlias(true); mtp.setColor(th.textSecondary);

    auto drawLine = [&](const std::string& line, float lineY) {
        if (line.empty()) return;
        SkRect mb;
        msgFont.measureText(line.c_str(), line.size(), SkTextEncoding::kUTF8, &mb);
        canvas->drawString(line.c_str(), dx + 70.f - mb.left(), lineY - mb.top(), msgFont, mtp);
    };

    auto nl = m_message.find('\n');
    if (nl != std::string::npos) {
        drawLine(m_message.substr(0, nl),       dy + kBarH + 60.f);
        drawLine(m_message.substr(nl + 1),      dy + kBarH + 78.f);
    } else {
        drawLine(m_message, dy + kBarH + 60.f);
    }

    // OK button
    const float bx = (float)btnX(), by_ = (float)btnY();
    SkRRect brr; brr.setRectXY(SkRect::MakeXYWH(bx, by_, kBtnW, kBtnH), 6.f, 6.f);
    SkPaint btnFill; btnFill.setAntiAlias(true);
    btnFill.setColor(m_okHover
        ? th.accentHover
        : th.accent);
    canvas->drawRRect(brr, btnFill);

    SkFont btnFont(tf, 13.f);
    btnFont.setEdging(SkFont::Edging::kAntiAlias);
    SkRect btb;
    btnFont.measureText("OK", 2, SkTextEncoding::kUTF8, &btb);
    SkPaint btnTp; btnTp.setAntiAlias(true); btnTp.setColor(SK_ColorWHITE);
    canvas->drawString("OK",
        bx + (kBtnW - btb.width()) * 0.5f - btb.left(),
        by_ + (kBtnH - btb.height()) * 0.5f - btb.top(),
        btnFont, btnTp);
}

bool skMessageBox::handleEvent(const skEvent& ev) {
    if (!visible()) return false;

    if (ev.type == skEventType::KeyDown && ev.button == VK_ESCAPE) {
        setVisible(false);
        if (m_onClose) m_onClose();
        return true;
    }

    if (ev.type == skEventType::MouseMove) {
        m_okHover = (ev.x >= btnX() && ev.x < btnX() + (int)kBtnW &&
                     ev.y >= btnY() && ev.y < btnY() + (int)kBtnH);
        return true; // consume all mouse moves (prevent widget interaction)
    }

    if (ev.type == skEventType::MouseDown) {
        if (ev.x >= btnX() && ev.x < btnX() + (int)kBtnW &&
            ev.y >= btnY() && ev.y < btnY() + (int)kBtnH) {
            setVisible(false);
            if (m_onClose) m_onClose();
        }
        return true; // always consume while visible
    }

    if (ev.type == skEventType::MouseUp) return true;

    return false;
}
