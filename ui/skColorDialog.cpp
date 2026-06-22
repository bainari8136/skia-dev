#include <Windows.h>
#include "skColorDialog.h"
#include "skTheme.h"
#include "skTypeface.h"
#include <include/core/SkPaint.h>
#include <include/core/SkFont.h>
#include <include/core/SkRRect.h>
#include <include/effects/SkGradientShader.h>
#include <algorithm>
#include <cmath>
#include <cstdio>

static sk_sp<SkTypeface>& cdTf() {
    static sk_sp<SkTypeface> tf = skGetSystemTypeface();
    return tf;
}

skColorDialog::skColorDialog(int winW, int winH)
    : skWidget(0, 0, winW, winH) {
    setVisible(false);
}

SkColor skColorDialog::hsvToRgb(float h, float s, float v) {
    h = std::fmod(h, 360.f); if (h < 0.f) h += 360.f;
    float c = v * s;
    float x = c * (1.f - std::fabsf(std::fmod(h / 60.f, 2.f) - 1.f));
    float m = v - c, r, g, b;
    if      (h < 60)  { r=c; g=x; b=0; }
    else if (h < 120) { r=x; g=c; b=0; }
    else if (h < 180) { r=0; g=c; b=x; }
    else if (h < 240) { r=0; g=x; b=c; }
    else if (h < 300) { r=x; g=0; b=c; }
    else              { r=c; g=0; b=x; }
    return SkColorSetRGB((uint8_t)((r+m)*255.f + 0.5f),
                         (uint8_t)((g+m)*255.f + 0.5f),
                         (uint8_t)((b+m)*255.f + 0.5f));
}

SkColor skColorDialog::currentColor() const { return hsvToRgb(m_hue, m_sat, m_val); }

void skColorDialog::show(SkColor c) {
    float r = SkColorGetR(c) / 255.f, g = SkColorGetG(c) / 255.f, b = SkColorGetB(c) / 255.f;
    float mx = std::max({r,g,b}), mn = std::min({r,g,b}), d = mx - mn;
    m_val = mx;
    m_sat = (mx == 0.f) ? 0.f : d / mx;
    if (d == 0.f) m_hue = 0.f;
    else if (mx == r) m_hue = 60.f * std::fmod((g-b)/d, 6.f);
    else if (mx == g) m_hue = 60.f * ((b-r)/d + 2.f);
    else              m_hue = 60.f * ((r-g)/d + 4.f);
    if (m_hue < 0.f) m_hue += 360.f;
    m_okHov = m_cancelHov = m_dragSV = m_dragHue = false;
    setVisible(true);
}

SkRect skColorDialog::svRect() const {
    return SkRect::MakeXYWH(dlgX() + 18.f, dlgY() + 54.f, kSVW, kSVH);
}
SkRect skColorDialog::hueRect() const {
    return SkRect::MakeXYWH(dlgX() + 18.f + kSVW + 10.f, dlgY() + 54.f, kHueW, kSVH);
}
SkRect skColorDialog::okRect() const {
    return SkRect::MakeXYWH(dlgX() + kDlgW - kBtnW - 14.f,
                             dlgY() + kDlgH - kBtnH - 14.f, kBtnW, kBtnH);
}
SkRect skColorDialog::cancelRect() const {
    SkRect ok = okRect();
    return SkRect::MakeXYWH(ok.left() - kBtnW - 8.f, ok.top(), kBtnW, kBtnH);
}

void skColorDialog::pickSV(int mx, int my) {
    SkRect r = svRect();
    m_sat = std::max(0.f, std::min(1.f, ((float)mx - r.left()) / r.width()));
    m_val = std::max(0.f, std::min(1.f, 1.f - ((float)my - r.top()) / r.height()));
}
void skColorDialog::pickHue(int my) {
    SkRect r = hueRect();
    m_hue = std::max(0.f, std::min(359.99f, ((float)my - r.top()) / r.height() * 360.f));
}

void skColorDialog::Paint(SkCanvas* canvas) {
    const auto& th = skGetTheme();
    float dx = dlgX(), dy = dlgY();

    // Backdrop
    SkPaint bkp; bkp.setColor(SkColorSetARGB(130, 0, 0, 0));
    canvas->drawRect(SkRect::MakeXYWH((float)x,(float)y,(float)w,(float)h), bkp);

    // Shadow
    SkRRect sh; sh.setRectXY(SkRect::MakeXYWH(dx+2,dy+6,(float)kDlgW,(float)kDlgH),kR,kR);
    SkPaint shp; shp.setAntiAlias(true); shp.setColor(SkColorSetARGB(28,0,0,0));
    canvas->drawRRect(sh, shp);

    // Dialog background
    SkRRect rr; rr.setRectXY(SkRect::MakeXYWH(dx,dy,(float)kDlgW,(float)kDlgH),kR,kR);
    SkPaint bg; bg.setAntiAlias(true); bg.setColor(th.panelBg);
    canvas->drawRRect(rr, bg);

    // Accent top bar
    canvas->save();
    canvas->clipRRect(rr, SkClipOp::kIntersect, true);
    SkPaint bar; bar.setColor(th.accent);
    canvas->drawRect(SkRect::MakeXYWH(dx,dy,(float)kDlgW,5.f),bar);
    canvas->restore();

    // Border
    SkPaint brd; brd.setAntiAlias(true);
    brd.setStyle(SkPaint::kStroke_Style); brd.setStrokeWidth(0.75f);
    brd.setColor(th.panelBorder);
    canvas->drawRRect(rr, brd);

    // Title
    {
        SkFont font(cdTf(), 14.f); font.setEdging(SkFont::Edging::kAntiAlias);
        const char* ttl = "Pick Colour";
        SkRect tb; font.measureText(ttl, strlen(ttl), SkTextEncoding::kUTF8, &tb);
        SkPaint tp; tp.setAntiAlias(true); tp.setColor(th.textPrimary);
        canvas->drawString(ttl, dx + 18.f - tb.left(), dy + 34.f - tb.top(), font, tp);
    }

    // SV square (saturation-value)
    SkRect svR = svRect();
    {
        SkColor hueCol = hsvToRgb(m_hue, 1.f, 1.f);
        // Layer 1: horizontal gradient white → hue
        SkPoint p1[2] = {{svR.left(),0},{svR.right(),0}};
        SkColor c1[2] = {SK_ColorWHITE, hueCol};
        SkPaint s1; s1.setShader(SkGradientShader::MakeLinear(p1,c1,nullptr,2,SkTileMode::kClamp));
        canvas->drawRect(svR, s1);
        // Layer 2: vertical gradient transparent → black
        SkPoint p2[2] = {{0,svR.top()},{0,svR.bottom()}};
        SkColor c2[2] = {SK_ColorTRANSPARENT, SK_ColorBLACK};
        SkPaint s2; s2.setShader(SkGradientShader::MakeLinear(p2,c2,nullptr,2,SkTileMode::kClamp));
        canvas->drawRect(svR, s2);

        // Crosshair
        float cx_ = svR.left() + m_sat * svR.width();
        float cy_ = svR.top()  + (1.f - m_val) * svR.height();
        SkPaint cur; cur.setAntiAlias(true);
        cur.setStyle(SkPaint::kStroke_Style); cur.setStrokeWidth(1.5f);
        cur.setColor(m_val > 0.5f ? SK_ColorBLACK : SK_ColorWHITE);
        canvas->drawCircle(cx_, cy_, 6.f, cur);

        SkPaint svBrd; svBrd.setStyle(SkPaint::kStroke_Style); svBrd.setStrokeWidth(1.f);
        svBrd.setColor(th.panelBorder);
        canvas->drawRect(svR, svBrd);
    }

    // Hue bar
    SkRect hueR = hueRect();
    {
        SkColor hc[] = {0xFFFF0000,0xFFFFFF00,0xFF00FF00,0xFF00FFFF,0xFF0000FF,0xFFFF00FF,0xFFFF0000};
        float   hp[] = {0.f,1.f/6,2.f/6,3.f/6,4.f/6,5.f/6,1.f};
        SkPoint pts[2] = {{0,hueR.top()},{0,hueR.bottom()}};
        SkPaint hb; hb.setShader(SkGradientShader::MakeLinear(pts,hc,hp,7,SkTileMode::kClamp));
        canvas->drawRect(hueR, hb);

        float hueY = hueR.top() + (m_hue / 360.f) * hueR.height();
        SkPaint hcp; hcp.setAntiAlias(true);
        hcp.setStyle(SkPaint::kStroke_Style); hcp.setStrokeWidth(2.f);
        hcp.setColor(SK_ColorWHITE);
        canvas->drawLine(hueR.left()-2.f, hueY, hueR.right()+2.f, hueY, hcp);

        SkPaint hBrd; hBrd.setStyle(SkPaint::kStroke_Style); hBrd.setStrokeWidth(1.f);
        hBrd.setColor(th.panelBorder);
        canvas->drawRect(hueR, hBrd);
    }

    // Color preview swatch
    SkColor cur = currentColor();
    float prevX = hueR.right() + 10.f;
    float prevY = hueR.top();
    float prevW = dx + (float)kDlgW - 14.f - prevX;
    SkRRect prr; prr.setRectXY(SkRect::MakeXYWH(prevX,prevY,prevW,44.f),4.f,4.f);
    SkPaint prp; prp.setAntiAlias(true); prp.setColor(cur);
    canvas->drawRRect(prr, prp);
    SkPaint pBrd; pBrd.setAntiAlias(true);
    pBrd.setStyle(SkPaint::kStroke_Style); pBrd.setStrokeWidth(1.f);
    pBrd.setColor(th.panelBorder);
    canvas->drawRRect(prr, pBrd);

    // Hex label
    {
        char hex[8]; snprintf(hex, sizeof(hex), "#%02X%02X%02X",
            SkColorGetR(cur), SkColorGetG(cur), SkColorGetB(cur));
        SkFont font(cdTf(), 11.f); font.setEdging(SkFont::Edging::kAntiAlias);
        SkRect hb; font.measureText(hex, strlen(hex), SkTextEncoding::kUTF8, &hb);
        SkPaint hp; hp.setAntiAlias(true); hp.setColor(th.textSecondary);
        canvas->drawString(hex,
            prevX + (prevW - hb.width()) / 2.f - hb.left(),
            prevY + 44.f + 12.f - hb.top(), font, hp);
    }

    // OK / Cancel buttons
    {
        SkFont btnFont(cdTf(), 13.f); btnFont.setEdging(SkFont::Edging::kAntiAlias);
        auto drawBtn = [&](SkRect r, const char* lbl, bool filled, bool hov) {
            SkRRect brr; brr.setRectXY(r, 5.f, 5.f);
            SkPaint bp; bp.setAntiAlias(true);
            bp.setColor(filled ? (hov ? th.accentHover : th.accent) : th.panelBg);
            canvas->drawRRect(brr, bp);
            SkPaint brd2; brd2.setAntiAlias(true);
            brd2.setStyle(SkPaint::kStroke_Style); brd2.setStrokeWidth(1.f);
            brd2.setColor(filled ? SkColorSetARGB(60,0,0,0) : (hov ? th.accent : th.inputBorder));
            canvas->drawRRect(brr, brd2);
            SkRect lb; btnFont.measureText(lbl, strlen(lbl), SkTextEncoding::kUTF8, &lb);
            SkPaint lp; lp.setAntiAlias(true);
            lp.setColor(filled ? th.textOnAccent : (hov ? th.accent : th.textPrimary));
            canvas->drawString(lbl,
                r.left()+(r.width()-lb.width())/2.f - lb.left(),
                r.top() +(r.height()-lb.height())/2.f - lb.top(),
                btnFont, lp);
        };
        drawBtn(okRect(),     "OK",     true,  m_okHov);
        drawBtn(cancelRect(), "Cancel", false, m_cancelHov);
    }
}

bool skColorDialog::handleEvent(const skEvent& ev) {
    if (!visible()) return false;

    if (ev.type == skEventType::KeyDown && ev.button == VK_ESCAPE) {
        setVisible(false); if (m_onCancel) m_onCancel(); return true;
    }

    SkRect svR = svRect(), hueR = hueRect(), okR = okRect(), canR = cancelRect();

    if (ev.type == skEventType::MouseMove) {
        m_okHov     = okR.contains((float)ev.x, (float)ev.y);
        m_cancelHov = canR.contains((float)ev.x, (float)ev.y);
        if (m_dragSV)  pickSV(ev.x, ev.y);
        if (m_dragHue) pickHue(ev.y);
        return true;
    }
    if (ev.type == skEventType::MouseDown) {
        if (svR.contains((float)ev.x, (float)ev.y))   { m_dragSV  = true; pickSV(ev.x, ev.y);  return true; }
        if (hueR.contains((float)ev.x, (float)ev.y))  { m_dragHue = true; pickHue(ev.y);        return true; }
        if (okR.contains((float)ev.x, (float)ev.y))   { setVisible(false); if (m_onConfirm) m_onConfirm(currentColor()); return true; }
        if (canR.contains((float)ev.x, (float)ev.y))  { setVisible(false); if (m_onCancel)  m_onCancel();               return true; }
        // Click outside dialog closes it
        float ddx = dlgX(), ddy = dlgY();
        bool inDlg = (ev.x>=(int)ddx && ev.x<(int)(ddx+kDlgW) && ev.y>=(int)ddy && ev.y<(int)(ddy+kDlgH));
        if (!inDlg) { setVisible(false); if (m_onCancel) m_onCancel(); }
        return true;
    }
    if (ev.type == skEventType::MouseUp) { m_dragSV = m_dragHue = false; return true; }
    return true;
}
