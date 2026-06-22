#include "skPdfView.h"
#include "skTheme.h"
#include "skTypeface.h"
#include <include/core/SkPaint.h>
#include <include/core/SkFont.h>
#include <include/core/SkCanvas.h>
#include <include/core/SkRRect.h>
#include <algorithm>
#include <cstdio>
#include <cstring>

static sk_sp<SkTypeface>& pvTf() {
    static sk_sp<SkTypeface> tf = skGetSystemTypeface();
    return tf;
}

skPdfView::skPdfView(int sx, int sy, int sw, int sh)
    : skWidget(sx, sy, sw, sh) {}

SkRect skPdfView::toolbar()  const { return SkRect::MakeXYWH((float)x,(float)y,(float)w,(float)kBarH); }
SkRect skPdfView::prevBtn()  const { return SkRect::MakeXYWH((float)(x+4),(float)(y+3),28.f,kBarH-6.f); }
SkRect skPdfView::nextBtn()  const { SkRect p=prevBtn(); return SkRect::MakeXYWH(p.right()+4.f,p.top(),28.f,p.height()); }
SkRect skPdfView::pageArea() const { return SkRect::MakeXYWH((float)x,(float)(y+kBarH),(float)w,(float)(h-kBarH)); }

void skPdfView::loadFile(const std::string& path) {
    m_filepath = path;
    m_filename = path;
    auto pos = path.find_last_of("/\\");
    if (pos != std::string::npos) m_filename = path.substr(pos+1);
    if (m_pageCount == 0) m_pageCount = 1;
    m_page = 1;
}

void skPdfView::goToPage(int page) {
    if (m_pageCount > 0) {
        m_page = std::max(1, std::min(m_pageCount, page));
        if (m_onPageChange) m_onPageChange(m_page);
    }
}

void skPdfView::setZoom(float z) {
    m_zoom = std::max(0.25f, std::min(4.f, z));
}

void skPdfView::Paint(SkCanvas* canvas) {
    const auto& th = skGetTheme();
    SkFont font(pvTf(), 11.f); font.setEdging(SkFont::Edging::kAntiAlias);
    SkFont small(pvTf(), 10.f); small.setEdging(SkFont::Edging::kAntiAlias);

    SkRRect outerRR; outerRR.setRectXY(SkRect::MakeXYWH((float)x,(float)y,(float)w,(float)h),6.f,6.f);
    canvas->save(); canvas->clipRRect(outerRR, SkClipOp::kIntersect, true);
    SkPaint bgP; bgP.setAntiAlias(true); bgP.setColor(th.panelBg); canvas->drawRRect(outerRR,bgP);

    // Toolbar
    SkPaint barP; barP.setColor(th.inputBg); canvas->drawRect(toolbar(),barP);

    auto drawNavBtn = [&](SkRect r, const char* sym, bool hov, bool en) {
        SkRRect br; br.setRectXY(r,3.f,3.f);
        if (hov && en) { SkPaint hp; hp.setAntiAlias(true); hp.setColor(SkColorSetARGB(40,128,128,128)); canvas->drawRRect(br,hp); }
        SkPaint tp; tp.setAntiAlias(true); tp.setColor(en ? th.textPrimary : th.textSecondary);
        SkRect tb; font.measureText(sym,strlen(sym),SkTextEncoding::kUTF8,&tb);
        canvas->drawString(sym,r.centerX()-tb.width()/2.f-tb.left(),r.centerY()-tb.height()/2.f-tb.top(),font,tp);
    };
    drawNavBtn(prevBtn(), "\xe2\x97\x80", m_prevHov, m_page > 1);                // ◀
    drawNavBtn(nextBtn(), "\xe2\x96\xb6", m_nextHov, m_page < m_pageCount);      // ▶

    // Page info + zoom
    char info[64];
    if (m_pageCount > 0)
        snprintf(info,sizeof(info),"Page %d / %d    Zoom: %.0f%%",m_page,m_pageCount,m_zoom*100.f);
    else
        snprintf(info,sizeof(info),"No document loaded");
    { SkPaint ip; ip.setAntiAlias(true); ip.setColor(th.textSecondary);
      canvas->drawString(info, nextBtn().right()+10.f, toolbar().centerY()+4.f, small, ip); }

    // Divider
    { SkPaint dp; dp.setColor(th.panelBorder);
      canvas->drawLine((float)x,(float)(y+kBarH),(float)(x+w),(float)(y+kBarH),dp); }

    // Page area
    SkRect pa = pageArea();
    { SkPaint pageBg; pageBg.setColor(SkColorSetRGB(200,200,205));
      canvas->drawRect(pa, pageBg); }

    if (m_pageCount > 0) {
        // Paper simulation
        float pw = pa.width() * 0.72f * m_zoom;
        float ph = pa.height() * 0.88f * m_zoom;
        float px = pa.centerX() - pw/2.f;
        float py = pa.centerY() - ph/2.f;
        SkPaint paperP; paperP.setAntiAlias(true); paperP.setColor(SK_ColorWHITE);
        SkRRect prr; prr.setRectXY(SkRect::MakeXYWH(px,py,pw,ph),2.f,2.f);
        SkPaint pshadow; pshadow.setAntiAlias(true); pshadow.setColor(SkColorSetARGB(60,0,0,0));
        SkRRect psh; psh.setRectXY(SkRect::MakeXYWH(px+3.f,py+3.f,pw,ph),2.f,2.f);
        canvas->drawRRect(psh,pshadow); canvas->drawRRect(prr,paperP);

        // Placeholder text lines on the page
        canvas->save(); canvas->clipRRect(prr, SkClipOp::kIntersect, true);
        SkPaint lp; lp.setColor(SkColorSetRGB(210,210,215));
        float lineY = py+24.f, lw = pw*0.8f, ls = 14.f;
        for (int i=0; i<8 && lineY+ls < py+ph-10.f; ++i, lineY+=ls+4.f) {
            float w2 = (i%4==3) ? lw*0.55f : (i%4==1) ? lw*0.9f : lw;
            canvas->drawRect(SkRect::MakeXYWH(px+pw*0.1f,lineY,w2,ls-4.f),lp);
        }
        // Page + filename text
        SkPaint fp; fp.setAntiAlias(true); fp.setColor(SkColorSetRGB(140,150,170));
        canvas->drawString(m_filename.empty() ? "document.pdf" : m_filename.c_str(),
            px+8.f, py+ph-12.f, small, fp);
        char pg[16]; snprintf(pg,sizeof(pg),"%d",m_page);
        SkRect pgb; small.measureText(pg,strlen(pg),SkTextEncoding::kUTF8,&pgb);
        canvas->drawString(pg, px+pw/2.f-pgb.width()/2.f, py+ph-12.f, small, fp);
        canvas->restore();
    } else {
        SkPaint np; np.setAntiAlias(true); np.setColor(SkColorSetRGB(120,130,150));
        canvas->drawString("No PDF loaded", pa.centerX()-40.f, pa.centerY(), font, np);
    }

    SkPaint brd; brd.setAntiAlias(true); brd.setStyle(SkPaint::kStroke_Style);
    brd.setStrokeWidth(0.75f); brd.setColor(th.panelBorder);
    canvas->drawRRect(outerRR,brd);
    canvas->restore();
}

void skPdfView::OnEvent(const skEvent& ev) {
    if (ev.type == skEventType::MouseMove) {
        m_prevHov = prevBtn().contains((float)ev.x,(float)ev.y);
        m_nextHov = nextBtn().contains((float)ev.x,(float)ev.y);
    }
    if (ev.type == skEventType::MouseDown) {
        if (prevBtn().contains((float)ev.x,(float)ev.y)) prevPage();
        if (nextBtn().contains((float)ev.x,(float)ev.y)) nextPage();
    }
    if (ev.type == skEventType::MouseWheel && contains(ev.x,ev.y)) {
        if (ev.button > 0) prevPage(); else nextPage();
    }
}
