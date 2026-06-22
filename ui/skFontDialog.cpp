#include <Windows.h>
#include "skFontDialog.h"
#include "skTheme.h"
#include "skTypeface.h"
#include <include/core/SkPaint.h>
#include <include/core/SkFont.h>
#include <include/core/SkTypeface.h>
#include <include/core/SkFontStyle.h>
#include <include/core/SkRRect.h>
#include <algorithm>
#include <cstdio>
#include <cstring>
#include <set>

static sk_sp<SkTypeface>& fdTf2() {
    static sk_sp<SkTypeface> tf = skGetSystemTypeface();
    return tf;
}

// EnumFontFamiliesEx callback
static int CALLBACK FontEnumProc(ENUMLOGFONTEXW* lpelf, NEWTEXTMETRICEXW*, DWORD, LPARAM lParam) {
    auto* out = reinterpret_cast<std::vector<std::string>*>(lParam);
    const wchar_t* nm = lpelf->elfLogFont.lfFaceName;
    if (nm[0] != L'@') { // skip vertical fonts
        int len = WideCharToMultiByte(CP_UTF8, 0, nm, -1, nullptr, 0, nullptr, nullptr);
        if (len > 1) {
            std::string utf8(len - 1, '\0');
            WideCharToMultiByte(CP_UTF8, 0, nm, -1, &utf8[0], len, nullptr, nullptr);
            out->push_back(utf8);
        }
    }
    return 1;
}

void skFontDialog::enumFonts() {
    m_fonts.clear();
    HDC hdc = GetDC(nullptr);
    LOGFONTW lf = {}; lf.lfCharSet = DEFAULT_CHARSET;
    std::vector<std::string> raw;
    EnumFontFamiliesExW(hdc, &lf, (FONTENUMPROCW)FontEnumProc, (LPARAM)&raw, 0);
    ReleaseDC(nullptr, hdc);
    // Deduplicate + sort
    std::set<std::string> seen;
    for (auto& f : raw)
        if (seen.insert(f).second) m_fonts.push_back(f);
    std::sort(m_fonts.begin(), m_fonts.end());
}

skFontDialog::skFontDialog(int winW, int winH)
    : skWidget(0, 0, winW, winH) {
    setVisible(false);
    enumFonts();
}

void skFontDialog::show(skFontSelection init) {
    m_sel = init;
    m_fontScroll = 0; m_okHov = m_cancelHov = false;
    // Scroll to selected font
    auto it = std::find(m_fonts.begin(), m_fonts.end(), m_sel.family);
    if (it != m_fonts.end()) {
        int idx = (int)(it - m_fonts.begin());
        ensureVisible(idx);
    }
    setVisible(true);
}

int skFontDialog::visibleFonts() const { return std::max(1, (int)(kListH / kRowH)); }
int skFontDialog::maxFontScroll() const { return std::max(0, (int)m_fonts.size() - visibleFonts()); }
void skFontDialog::ensureVisible(int idx) {
    if (idx < m_fontScroll) m_fontScroll = idx;
    if (idx >= m_fontScroll + visibleFonts()) m_fontScroll = idx - visibleFonts() + 1;
}

SkRect skFontDialog::listRect() const {
    return SkRect::MakeXYWH(dlgX()+8.f, dlgY()+kListY, kListW, kListH);
}
SkRect skFontDialog::previewRect() const {
    float ry = dlgY()+kListY+kListH+8.f;
    return SkRect::MakeXYWH(dlgX()+8.f, ry, (float)kDlgW-16.f, kPrevH);
}
SkRect skFontDialog::okRect() const {
    return SkRect::MakeXYWH(dlgX()+kDlgW-kBtnW-10.f, dlgY()+kDlgH-kBtnH-10.f, kBtnW, kBtnH);
}
SkRect skFontDialog::cancelRect() const {
    SkRect ok = okRect();
    return SkRect::MakeXYWH(ok.left()-kBtnW-8.f, ok.top(), kBtnW, kBtnH);
}
SkRect skFontDialog::boldRect() const {
    return SkRect::MakeXYWH(dlgX()+kListW+16.f, dlgY()+kListY, 90.f, 24.f);
}
SkRect skFontDialog::italicRect() const {
    SkRect b = boldRect();
    return SkRect::MakeXYWH(b.left(), b.bottom()+8.f, 90.f, 24.f);
}
SkRect skFontDialog::sizeUpRect() const {
    return SkRect::MakeXYWH(dlgX()+kListW+16.f+60.f, dlgY()+kListY+70.f, 28.f, 28.f);
}
SkRect skFontDialog::sizeDnRect() const {
    return SkRect::MakeXYWH(dlgX()+kListW+16.f+28.f, dlgY()+kListY+70.f, 28.f, 28.f);
}

int skFontDialog::fontAt(int py) const {
    SkRect lr = listRect();
    if (py < (int)lr.top() || py >= (int)lr.bottom()) return -1;
    int idx = (int)((float)(py-(int)lr.top()) / kRowH) + m_fontScroll;
    return (idx >= 0 && idx < (int)m_fonts.size()) ? idx : -1;
}

void skFontDialog::Paint(SkCanvas* canvas) {
    const auto& th = skGetTheme();
    float dx = dlgX(), dy = dlgY();

    // Backdrop
    SkPaint bk; bk.setColor(SkColorSetARGB(130,0,0,0));
    canvas->drawRect(SkRect::MakeXYWH((float)x,(float)y,(float)w,(float)h), bk);

    // Dialog
    SkRRect rr; rr.setRectXY(SkRect::MakeXYWH(dx,dy,(float)kDlgW,(float)kDlgH), kR, kR);
    SkPaint bgP; bgP.setAntiAlias(true); bgP.setColor(th.panelBg);
    canvas->drawRRect(rr, bgP);
    canvas->save(); canvas->clipRRect(rr, SkClipOp::kIntersect, true);
    SkPaint barP; barP.setColor(th.accent);
    canvas->drawRect(SkRect::MakeXYWH(dx,dy,(float)kDlgW,5.f), barP);
    canvas->restore();
    SkPaint brd; brd.setAntiAlias(true); brd.setStyle(SkPaint::kStroke_Style);
    brd.setStrokeWidth(0.75f); brd.setColor(th.panelBorder);
    canvas->drawRRect(rr, brd);

    SkFont font(fdTf2(), 12.f);  font.setEdging(SkFont::Edging::kAntiAlias);
    SkFont small(fdTf2(), 11.f); small.setEdging(SkFont::Edging::kAntiAlias);

    // Title
    { SkPaint tp; tp.setAntiAlias(true); tp.setColor(th.textPrimary);
      canvas->drawString("Pick Font", dx+14.f, dy+28.f, font, tp); }

    // Font list
    SkRect lr = listRect();
    canvas->save(); canvas->clipRect(lr);
    int end = std::min((int)m_fonts.size(), m_fontScroll + visibleFonts());
    for (int i = m_fontScroll; i < end; ++i) {
        float ry = lr.top() + (float)(i-m_fontScroll)*kRowH;
        bool sel = (m_fonts[i] == m_sel.family);
        if (sel) { SkPaint sp; sp.setColor(SkColorSetARGB(50, SkColorGetR(th.accent),
            SkColorGetG(th.accent), SkColorGetB(th.accent)));
            canvas->drawRect(SkRect::MakeXYWH(lr.left(),ry,lr.width()-kSbW,kRowH),sp); }
        SkPaint np; np.setAntiAlias(true); np.setColor(sel ? th.accent : th.textPrimary);
        canvas->drawString(m_fonts[i].c_str(), lr.left()+4.f, ry+kRowH-5.f, small, np);
    }
    canvas->restore();
    { SkPaint lbrd; lbrd.setAntiAlias(true);
      lbrd.setStyle(SkPaint::kStroke_Style); lbrd.setStrokeWidth(1.f);
      lbrd.setColor(th.inputBorder); canvas->drawRect(lr, lbrd); }

    // Scrollbar
    int ms = maxFontScroll();
    if (ms > 0) {
        float sbX = lr.right()-kSbW;
        float ratio = (float)visibleFonts()/(float)(visibleFonts()+ms);
        float thumbH = std::max(16.f, kListH*ratio);
        float thumbY = lr.top()+(float)m_fontScroll/(float)ms*(kListH-thumbH);
        SkRRect tRR; tRR.setRectXY(SkRect::MakeXYWH(sbX+1.f,thumbY+1.f,kSbW-2.f,thumbH-2.f),3.f,3.f);
        SkPaint tP; tP.setAntiAlias(true); tP.setColor(th.inputBorder);
        canvas->drawRRect(tRR, tP);
    }

    // Right panel: Bold, Italic, Size
    auto drawCheck = [&](SkRect r, const char* lbl, bool checked) {
        SkPaint cp; cp.setAntiAlias(true);
        cp.setStyle(SkPaint::kStroke_Style); cp.setStrokeWidth(1.f);
        cp.setColor(checked ? th.accent : th.inputBorder);
        canvas->drawRect(SkRect::MakeXYWH(r.left(),r.centerY()-7.f,14.f,14.f), cp);
        if (checked) {
            SkPaint fp; fp.setAntiAlias(true); fp.setColor(th.accent);
            canvas->drawRect(SkRect::MakeXYWH(r.left()+3.f,r.centerY()-4.f,8.f,8.f),fp);
        }
        SkPaint lp; lp.setAntiAlias(true); lp.setColor(th.textPrimary);
        canvas->drawString(lbl, r.left()+18.f, r.centerY()+5.f, small, lp);
    };
    drawCheck(boldRect(),   "Bold",   m_sel.bold);
    drawCheck(italicRect(), "Italic", m_sel.italic);

    // Size controls
    { SkPaint lp; lp.setAntiAlias(true); lp.setColor(th.textSecondary);
      canvas->drawString("Size:", boldRect().left(), boldRect().bottom()+42.f, small, lp); }
    char szBuf[8]; snprintf(szBuf,sizeof(szBuf),"%d",m_sel.size);
    { SkRect tb; font.measureText(szBuf,strlen(szBuf),SkTextEncoding::kUTF8,&tb);
      SkPaint vp; vp.setAntiAlias(true); vp.setColor(th.textPrimary);
      canvas->drawString(szBuf, boldRect().left(), boldRect().bottom()+64.f, font, vp); }

    auto drawArrow = [&](SkRect r, const char* s) {
        SkRRect brr; brr.setRectXY(r,4.f,4.f);
        SkPaint bp; bp.setAntiAlias(true); bp.setColor(th.panelBorder);
        canvas->drawRRect(brr, bp);
        SkRect tb; small.measureText(s,strlen(s),SkTextEncoding::kUTF8,&tb);
        SkPaint tp2; tp2.setAntiAlias(true); tp2.setColor(th.textPrimary);
        canvas->drawString(s,r.centerX()-tb.width()/2.f-tb.left(),
                           r.centerY()-tb.height()/2.f-tb.top(),small,tp2);
    };
    drawArrow(sizeUpRect(), "+");
    drawArrow(sizeDnRect(), "-");

    // Preview
    SkRect pr = previewRect();
    { SkPaint prBg; prBg.setColor(th.inputBg); canvas->drawRect(pr,prBg);
      SkPaint prBrd; prBrd.setAntiAlias(true);
      prBrd.setStyle(SkPaint::kStroke_Style); prBrd.setStrokeWidth(1.f);
      prBrd.setColor(th.inputBorder); canvas->drawRect(pr,prBrd); }
    {
        // Render preview text — use system typeface (no MakeFromName in this Skia build)
        auto prevTf = fdTf2();
        SkFont prevFont(prevTf, (float)m_sel.size);
        prevFont.setEdging(SkFont::Edging::kAntiAlias);
        SkPaint pp; pp.setAntiAlias(true); pp.setColor(th.textPrimary);
        canvas->save(); canvas->clipRect(pr);
        canvas->drawString("AaBbCcDd 0123",
            pr.left()+6.f, pr.centerY()+5.f, prevFont, pp);
        canvas->restore();
    }
    // Selected font name below preview
    { SkPaint fp; fp.setAntiAlias(true); fp.setColor(th.textSecondary);
      std::string info = m_sel.family + "  " + std::to_string(m_sel.size) + "pt";
      if (m_sel.bold)   info += " Bold";
      if (m_sel.italic) info += " Italic";
      canvas->drawString(info.c_str(), pr.left(), pr.bottom()+14.f, small, fp); }

    // OK / Cancel
    auto drawBtn = [&](SkRect r, const char* lbl, bool filled, bool hov) {
        SkRRect brr; brr.setRectXY(r,5.f,5.f);
        SkPaint bp; bp.setAntiAlias(true);
        bp.setColor(filled?(hov?th.accentHover:th.accent):th.panelBg);
        canvas->drawRRect(brr,bp);
        SkPaint brd2; brd2.setAntiAlias(true);
        brd2.setStyle(SkPaint::kStroke_Style); brd2.setStrokeWidth(1.f);
        brd2.setColor(filled?SkColorSetARGB(60,0,0,0):(hov?th.accent:th.inputBorder));
        canvas->drawRRect(brr,brd2);
        SkRect lb; small.measureText(lbl,strlen(lbl),SkTextEncoding::kUTF8,&lb);
        SkPaint lp; lp.setAntiAlias(true);
        lp.setColor(filled?th.textOnAccent:(hov?th.accent:th.textPrimary));
        canvas->drawString(lbl,r.centerX()-lb.width()/2.f-lb.left(),
                           r.centerY()-lb.height()/2.f-lb.top(),small,lp);
    };
    drawBtn(okRect(),     "OK",     true,  m_okHov);
    drawBtn(cancelRect(), "Cancel", false, m_cancelHov);
}

bool skFontDialog::handleEvent(const skEvent& ev) {
    if (!visible()) return false;
    if (ev.type == skEventType::KeyDown && ev.button == VK_ESCAPE) {
        setVisible(false); if (m_onCancel) m_onCancel(); return true;
    }
    SkRect lr = listRect(), okR = okRect(), canR = cancelRect();
    SkRect bolR = boldRect(), itR = italicRect(), suR = sizeUpRect(), sdR = sizeDnRect();

    if (ev.type == skEventType::MouseMove) {
        m_okHov     = okR.contains((float)ev.x,(float)ev.y);
        m_cancelHov = canR.contains((float)ev.x,(float)ev.y);
        m_hoverFont = fontAt(ev.y);
        return true;
    }
    if (ev.type == skEventType::MouseWheel && lr.contains((float)ev.x,(float)ev.y)) {
        if (ev.button > 0) m_fontScroll = std::max(0, m_fontScroll-2);
        else               m_fontScroll = std::min(maxFontScroll(), m_fontScroll+2);
        return true;
    }
    if (ev.type == skEventType::MouseDown) {
        if (okR.contains((float)ev.x,(float)ev.y))  { setVisible(false); if(m_onConfirm)m_onConfirm(m_sel); return true; }
        if (canR.contains((float)ev.x,(float)ev.y)) { setVisible(false); if(m_onCancel)m_onCancel();         return true; }
        if (bolR.contains((float)ev.x,(float)ev.y)) { m_sel.bold   = !m_sel.bold;   return true; }
        if (itR.contains((float)ev.x,(float)ev.y))  { m_sel.italic = !m_sel.italic; return true; }
        if (suR.contains((float)ev.x,(float)ev.y))  { if(m_sel.size<72) ++m_sel.size; return true; }
        if (sdR.contains((float)ev.x,(float)ev.y))  { if(m_sel.size>6)  --m_sel.size; return true; }
        if (lr.contains((float)ev.x,(float)ev.y)) {
            int idx = fontAt(ev.y);
            if (idx >= 0) m_sel.family = m_fonts[idx];
            return true;
        }
        // Outside dialog
        float ddx=dlgX(),ddy=dlgY();
        bool in=(ev.x>=(int)ddx&&ev.x<(int)(ddx+kDlgW)&&ev.y>=(int)ddy&&ev.y<(int)(ddy+kDlgH));
        if (!in) { setVisible(false); if(m_onCancel)m_onCancel(); }
        return true;
    }
    return true;
}
