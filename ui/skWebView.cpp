#include <Windows.h>
#include "skWebView.h"
#include "skTheme.h"
#include "skTypeface.h"
#include <include/core/SkPaint.h>
#include <include/core/SkFont.h>
#include <include/core/SkCanvas.h>
#include <include/core/SkRRect.h>
#include <algorithm>
#include <cstring>

static sk_sp<SkTypeface>& wvTf() {
    static sk_sp<SkTypeface> tf = skGetSystemTypeface();
    return tf;
}

skWebView::skWebView(int sx, int sy, int sw, int sh)
    : skWidget(sx, sy, sw, sh) {
    m_editBuf = m_url;
}

SkRect skWebView::barRect()  const { return SkRect::MakeXYWH((float)x,(float)y,(float)w,(float)kBarH); }
SkRect skWebView::backBtn()  const { return SkRect::MakeXYWH((float)(x+4),(float)(y+3),(float)kBtnW,kBarH-6.f); }
SkRect skWebView::fwdBtn()   const { SkRect b=backBtn(); return SkRect::MakeXYWH(b.right()+2.f,b.top(),(float)kBtnW,b.height()); }
SkRect skWebView::goBtn()    const { return SkRect::MakeXYWH((float)(x+w-kBtnW-4),(float)(y+3),(float)kBtnW,kBarH-6.f); }
SkRect skWebView::urlRect()  const { SkRect f=fwdBtn(),g=goBtn(); return SkRect::MakeXYWH(f.right()+4.f,(float)(y+3),g.left()-f.right()-8.f,kBarH-6.f); }
SkRect skWebView::bodyRect() const { return SkRect::MakeXYWH((float)x,(float)(y+kBarH),(float)w,(float)(h-kBarH)); }

void skWebView::navigate(const std::string& url) {
    m_url    = url;
    m_editBuf = url;
    m_urlFocused = false;
    if (m_histIdx < (int)m_history.size()-1)
        m_history.erase(m_history.begin()+m_histIdx+1, m_history.end());
    m_history.push_back(url);
    m_histIdx = (int)m_history.size()-1;
    if (m_onNavigate) m_onNavigate(url);
}

void skWebView::back() {
    if (m_histIdx > 0) { m_url = m_history[--m_histIdx]; m_editBuf = m_url; }
}
void skWebView::forward() {
    if (m_histIdx < (int)m_history.size()-1) { m_url = m_history[++m_histIdx]; m_editBuf = m_url; }
}
void skWebView::refresh() { if (m_onNavigate) m_onNavigate(m_url); }

void skWebView::Paint(SkCanvas* canvas) {
    const auto& th = skGetTheme();
    SkFont font(wvTf(), 11.f); font.setEdging(SkFont::Edging::kAntiAlias);
    SkFont small(wvTf(), 10.f); small.setEdging(SkFont::Edging::kAntiAlias);

    // Border + bg
    SkRRect rr; rr.setRectXY(SkRect::MakeXYWH((float)x,(float)y,(float)w,(float)h),6.f,6.f);
    canvas->save(); canvas->clipRRect(rr, SkClipOp::kIntersect, true);
    SkPaint bgP; bgP.setColor(th.panelBg); canvas->drawRRect(rr,bgP);

    // Navigation bar
    SkPaint barP; barP.setColor(th.inputBg); canvas->drawRect(barRect(),barP);

    auto drawNavBtn = [&](SkRect r, const char* sym, bool hov, bool enabled) {
        if (hov && enabled) {
            SkPaint hp; hp.setAntiAlias(true); hp.setColor(SkColorSetARGB(40,128,128,128));
            SkRRect br; br.setRectXY(r,3.f,3.f); canvas->drawRRect(br,hp);
        }
        SkPaint tp; tp.setAntiAlias(true);
        tp.setColor(enabled ? th.textPrimary : th.textSecondary);
        SkRect tb; font.measureText(sym,strlen(sym),SkTextEncoding::kUTF8,&tb);
        canvas->drawString(sym,r.centerX()-tb.width()/2.f-tb.left(),r.centerY()-tb.height()/2.f-tb.top(),font,tp);
    };
    drawNavBtn(backBtn(), "\xe2\x86\x90", m_backHov, m_histIdx > 0);      // ←
    drawNavBtn(fwdBtn(),  "\xe2\x86\x92", m_fwdHov,  m_histIdx < (int)m_history.size()-1); // →

    // URL box
    SkRect ur = urlRect();
    SkPaint urlBg; urlBg.setColor(th.inputBg);
    SkPaint urlBrd; urlBrd.setAntiAlias(true); urlBrd.setStyle(SkPaint::kStroke_Style);
    urlBrd.setStrokeWidth(1.f); urlBrd.setColor(m_urlFocused ? th.accent : th.inputBorder);
    SkRRect urr; urr.setRectXY(ur,3.f,3.f);
    canvas->drawRRect(urr,urlBg); canvas->drawRRect(urr,urlBrd);
    canvas->save(); canvas->clipRect(ur);
    SkPaint urlTp; urlTp.setAntiAlias(true); urlTp.setColor(th.textPrimary);
    canvas->drawString(m_urlFocused ? m_editBuf.c_str() : m_url.c_str(),
        ur.left()+6.f, ur.centerY()+4.f, small, urlTp);
    if (m_urlFocused) {
        SkRect cb; small.measureText(m_editBuf.c_str(),m_editBuf.size(),SkTextEncoding::kUTF8,&cb);
        float cx = ur.left()+6.f+cb.width();
        SkPaint cp; cp.setColor(th.accent);
        canvas->drawLine(cx,ur.top()+4.f,cx,ur.bottom()-4.f,cp);
    }
    canvas->restore();

    drawNavBtn(goBtn(), "\xe2\x86\x92", m_goHov, true); // → (Go)

    // Body area
    SkRect body = bodyRect();
    SkPaint bodyBg; bodyBg.setColor(SkColorSetRGB(250,250,252));
    canvas->drawRect(body, bodyBg);
    canvas->save(); canvas->clipRect(body);
    // Placeholder content
    SkPaint placP; placP.setAntiAlias(true); placP.setColor(SkColorSetRGB(100,120,160));
    canvas->drawString(m_url == "about:blank" ? "about:blank" : ("Navigated to: " + m_url).c_str(),
        body.left()+12.f, body.top()+26.f, font, placP);
    SkPaint hintP; hintP.setAntiAlias(true); hintP.setColor(SkColorSetRGB(180,180,200));
    canvas->drawString("Web rendering not available in this build.",
        body.left()+12.f, body.top()+50.f, small, hintP);
    canvas->restore();

    // Divider below bar
    SkPaint divP; divP.setColor(th.panelBorder);
    canvas->drawLine((float)x,(float)(y+kBarH),(float)(x+w),(float)(y+kBarH),divP);

    // Outer border
    SkPaint brd; brd.setAntiAlias(true); brd.setStyle(SkPaint::kStroke_Style);
    brd.setStrokeWidth(0.75f); brd.setColor(th.panelBorder);
    canvas->drawRRect(rr,brd);
    canvas->restore();
}

void skWebView::OnEvent(const skEvent& ev) {
    if (ev.type == skEventType::MouseMove) {
        m_backHov = backBtn().contains((float)ev.x,(float)ev.y);
        m_fwdHov  = fwdBtn().contains((float)ev.x,(float)ev.y);
        m_goHov   = goBtn().contains((float)ev.x,(float)ev.y);
    }
    if (ev.type == skEventType::MouseDown) {
        if (backBtn().contains((float)ev.x,(float)ev.y)) { back(); return; }
        if (fwdBtn().contains((float)ev.x,(float)ev.y))  { forward(); return; }
        if (goBtn().contains((float)ev.x,(float)ev.y))   { navigate(m_editBuf); return; }
        m_urlFocused = urlRect().contains((float)ev.x,(float)ev.y);
        if (m_urlFocused) m_editBuf = m_url;
    }
    if (ev.type == skEventType::KeyChar && m_urlFocused) {
        if (ev.ch >= 32) m_editBuf += (char)ev.ch;
    }
    if (ev.type == skEventType::KeyDown && m_urlFocused) {
        if (ev.button == VK_BACK && !m_editBuf.empty()) m_editBuf.pop_back();
        if (ev.button == VK_RETURN) { navigate(m_editBuf); }
        if (ev.button == VK_ESCAPE) { m_urlFocused = false; m_editBuf = m_url; }
    }
}
