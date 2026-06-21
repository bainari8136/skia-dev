#include <Windows.h>
#include "skModal.h"
#include "skTypeface.h"
#include "skTheme.h"
#include <include/core/SkPaint.h>
#include <include/core/SkFont.h>
#include <include/core/SkRRect.h>

static constexpr float kDlgW = 400.f;
static constexpr float kDlgH = 190.f;
static constexpr float kBtnW = 110.f;
static constexpr float kBtnH =  36.f;

skModal::skModal(int mx, int my, int mw, int mh)
    : skWidget(mx, my, mw, mh) {}

void skModal::show(const std::string& title, const std::string& message) {
    m_title   = title;
    m_message = message;
    m_confirmHov = m_cancelHov = false;
    setVisible(true);
}

SkRect skModal::dialogRect() const {
    float dx = (float)x + ((float)w - kDlgW) / 2.f;
    float dy = (float)y + ((float)h - kDlgH) / 2.f;
    return SkRect::MakeXYWH(dx, dy, kDlgW, kDlgH);
}
SkRect skModal::confirmRect() const {
    SkRect d = dialogRect();
    return SkRect::MakeXYWH(d.right() - kBtnW - 16.f, d.bottom() - kBtnH - 16.f, kBtnW, kBtnH);
}
SkRect skModal::cancelRect() const {
    SkRect d  = dialogRect();
    SkRect cr = confirmRect();
    return SkRect::MakeXYWH(cr.left() - kBtnW - 10.f, cr.top(), kBtnW, kBtnH);
}

void skModal::Paint(SkCanvas* canvas) {
    if (!visible()) return;
    const auto& th = skGetTheme();

    // Semi-transparent backdrop
    SkPaint backdrop;
    backdrop.setColor(SkColorSetARGB(130, 0, 0, 0));
    canvas->drawRect(SkRect::MakeXYWH((float)x, (float)y, (float)w, (float)h), backdrop);

    // Dialog box
    SkRect d = dialogRect();
    SkRRect dlgRR;
    dlgRR.setRectXY(d, 12.f, 12.f);

    SkPaint dlgBg;
    dlgBg.setAntiAlias(true);
    dlgBg.setColor(th.panelBg);
    canvas->drawRRect(dlgRR, dlgBg);

    SkPaint dlgBrd;
    dlgBrd.setAntiAlias(true);
    dlgBrd.setStyle(SkPaint::kStroke_Style);
    dlgBrd.setStrokeWidth(1.f);
    dlgBrd.setColor(th.panelBorder);
    canvas->drawRRect(dlgRR, dlgBrd);

    // Accent top bar
    canvas->save();
    canvas->clipRRect(dlgRR, SkClipOp::kIntersect, true);
    SkPaint topBar;
    topBar.setColor(th.accent);
    canvas->drawRect(SkRect::MakeXYWH(d.left(), d.top(), d.width(), 4.f), topBar);
    canvas->restore();

    static sk_sp<SkTypeface> s_tf = skGetSystemTypeface();
    const float padX = 24.f;

    // Title
    {
        SkFont font(s_tf, 16.f);
        font.setEdging(SkFont::Edging::kAntiAlias);
        SkRect tb;
        font.measureText(m_title.c_str(), m_title.size(), SkTextEncoding::kUTF8, &tb);
        SkPaint tp;
        tp.setAntiAlias(true);
        tp.setColor(th.textPrimary);
        canvas->drawString(m_title.c_str(), d.left() + padX - tb.left(), d.top() + 28.f - tb.top(), font, tp);
    }

    // Message
    {
        SkFont font(s_tf, 13.f);
        font.setEdging(SkFont::Edging::kAntiAlias);
        SkRect mb;
        font.measureText(m_message.c_str(), m_message.size(), SkTextEncoding::kUTF8, &mb);
        SkPaint mp;
        mp.setAntiAlias(true);
        mp.setColor(th.textSecondary);
        canvas->drawString(m_message.c_str(), d.left() + padX - mb.left(), d.top() + 70.f - mb.top(), font, mp);
    }

    // Divider
    {
        SkPaint div;
        div.setColor(th.panelBorder);
        canvas->drawLine(d.left(), d.bottom() - kBtnH - 28.f,
                         d.right(), d.bottom() - kBtnH - 28.f, div);
    }

    // Buttons
    SkFont btnFont(s_tf, 14.f);
    btnFont.setEdging(SkFont::Edging::kAntiAlias);

    auto drawBtn = [&](SkRect r, const char* label, bool filled, bool hovered) {
        SkRRect rr;
        rr.setRectXY(r, 6.f, 6.f);
        SkColor bg = filled ? (hovered ? th.accentHover : th.accent) : th.panelBg;
        SkPaint bp;
        bp.setAntiAlias(true);
        bp.setColor(bg);
        canvas->drawRRect(rr, bp);

        SkPaint brd;
        brd.setAntiAlias(true);
        brd.setStyle(SkPaint::kStroke_Style);
        brd.setStrokeWidth(1.f);
        brd.setColor(filled ? SkColorSetARGB(60, 0, 0, 0) : (hovered ? th.accent : th.inputBorder));
        canvas->drawRRect(rr, brd);

        SkRect lb;
        btnFont.measureText(label, strlen(label), SkTextEncoding::kUTF8, &lb);
        SkPaint lp;
        lp.setAntiAlias(true);
        lp.setColor(filled ? th.textOnAccent : (hovered ? th.accent : th.textPrimary));
        canvas->drawString(label,
            r.left() + (r.width() - lb.width()) / 2.f - lb.left(),
            r.top()  + (r.height() - lb.height()) / 2.f - lb.top(),
            btnFont, lp);
    };

    drawBtn(confirmRect(), "Confirm", true,  m_confirmHov);
    drawBtn(cancelRect(),  "Cancel",  false, m_cancelHov);
}

bool skModal::handleEvent(const skEvent& ev) {
    if (!visible()) return false;

    if (ev.type == skEventType::MouseMove) {
        m_confirmHov = confirmRect().contains((float)ev.x, (float)ev.y);
        m_cancelHov  = cancelRect() .contains((float)ev.x, (float)ev.y);
        return true;
    }
    if (ev.type == skEventType::MouseDown) {
        if (confirmRect().contains((float)ev.x, (float)ev.y)) {
            setVisible(false);
            if (m_onConfirm) m_onConfirm();
        } else if (cancelRect().contains((float)ev.x, (float)ev.y)) {
            setVisible(false);
            if (m_onCancel) m_onCancel();
        }
        return true; // consume all clicks while visible
    }
    if (ev.type == skEventType::KeyDown && ev.button == VK_ESCAPE) {
        setVisible(false);
        if (m_onCancel) m_onCancel();
        return true;
    }
    return true; // block everything else too
}
