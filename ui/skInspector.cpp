#include "skInspector.h"
#include "skTheme.h"
#include "skTypeface.h"
#include <include/core/SkPaint.h>
#include <include/core/SkFont.h>
#include <include/core/SkCanvas.h>
#include <include/core/SkRRect.h>
#include <cstdio>
#include <algorithm>

static sk_sp<SkTypeface>& insTf() {
    static sk_sp<SkTypeface> tf = skGetSystemTypeface();
    return tf;
}

static sk_sp<SkTypeface>& insMono() {
    static sk_sp<SkTypeface> tf = skGetSystemTypeface();
    return tf;
}

skInspector::skInspector(int sx, int sy, int sw, int sh)
    : skWidget(sx, sy, sw, sh) {}

void skInspector::Paint(SkCanvas* canvas) {
    const auto& th = skGetTheme();

    struct TokenEntry { const char* name; SkColor color; };
    const TokenEntry tokens[] = {
        { "accent",        th.accent        },
        { "accentHover",   th.accentHover   },
        { "accentPress",   th.accentPress   },
        { "textPrimary",   th.textPrimary   },
        { "textSecondary", th.textSecondary },
        { "textOnAccent",  th.textOnAccent  },
        { "panelBg",       th.panelBg       },
        { "panelBorder",   th.panelBorder   },
        { "inputBg",       th.inputBg       },
        { "inputBorder",   th.inputBorder   },
        { "trackBg",       th.trackBg       },
        { "windowBg",      th.windowBg      },
    };
    constexpr int N = (int)(sizeof(tokens)/sizeof(tokens[0]));

    // Background + border
    SkPaint bgP; bgP.setColor(th.panelBg);
    canvas->drawRect(SkRect::MakeXYWH((float)x,(float)y,(float)w,(float)h), bgP);
    canvas->save();
    canvas->clipRect(SkRect::MakeXYWH((float)x,(float)y,(float)w,(float)h));

    // Title bar
    SkPaint barP; barP.setColor(th.accent);
    canvas->drawRect(SkRect::MakeXYWH((float)x,(float)y,(float)w,4.f),barP);

    SkFont titleFont(insTf(), 11.f); titleFont.setEdging(SkFont::Edging::kAntiAlias);
    SkFont monoFont(insMono() ? insMono() : insTf(), 11.f); monoFont.setEdging(SkFont::Edging::kAntiAlias);
    SkPaint tp; tp.setAntiAlias(true); tp.setColor(th.textPrimary);
    SkPaint t2; t2.setAntiAlias(true); t2.setColor(th.textSecondary);

    // Column header
    float headerY = (float)(y+4+kPad);
    canvas->drawString("Token", (float)(x+kPad+kSwSz+8), headerY+10.f, titleFont, t2);
    canvas->drawString("Value",  (float)(x+kPad+kSwSz+8+120), headerY+10.f, titleFont, t2);
    {   SkPaint divP; divP.setColor(th.panelBorder); divP.setStrokeWidth(1.f);
        canvas->drawLine((float)x, headerY+18.f, (float)(x+w), headerY+18.f, divP); }

    float listY0 = headerY + 22.f;
    int visRows  = std::max(1,(int)((float)h - (listY0-(float)y)) / kRowH);
    int maxScr   = std::max(0, N - visRows);
    m_scroll     = std::min(m_scroll, maxScr);

    for (int i = 0; i < visRows && m_scroll+i < N; ++i) {
        const auto& tk = tokens[m_scroll+i];
        float ry = listY0 + (float)(i*kRowH);
        bool hov = (m_scroll+i == m_hover);

        if (hov) {
            SkPaint hp; hp.setColor(SkColorSetARGB(25, SkColorGetR(th.accent),
                SkColorGetG(th.accent), SkColorGetB(th.accent)));
            canvas->drawRect(SkRect::MakeXYWH((float)x,ry,(float)(w-kSbW),(float)kRowH),hp);
        }

        // Swatch
        float sx2 = (float)(x+kPad);
        float sy2 = ry + (kRowH-kSwSz)/2.f;
        SkRRect swr; swr.setRectXY(SkRect::MakeXYWH(sx2,sy2,(float)kSwSz,(float)kSwSz),3.f,3.f);
        SkPaint swP; swP.setAntiAlias(true); swP.setColor(tk.color);
        canvas->drawRRect(swr,swP);
        // Swatch border
        SkPaint swBrd; swBrd.setAntiAlias(true); swBrd.setStyle(SkPaint::kStroke_Style);
        swBrd.setStrokeWidth(0.75f); swBrd.setColor(SkColorSetARGB(60,0,0,0));
        canvas->drawRRect(swr,swBrd);

        // Token name
        float nx = sx2+(float)kSwSz+8.f;
        canvas->drawString(tk.name, nx, ry+(float)kRowH*0.65f, titleFont, tp);

        // Hex value
        char hex[12];
        snprintf(hex,sizeof(hex),"#%02X%02X%02X%02X",
            SkColorGetA(tk.color),SkColorGetR(tk.color),
            SkColorGetG(tk.color),SkColorGetB(tk.color));
        SkPaint hexP; hexP.setAntiAlias(true); hexP.setColor(th.accent);
        canvas->drawString(hex, nx+120.f, ry+(float)kRowH*0.65f, monoFont, hexP);
    }

    // Scrollbar
    if (maxScr > 0) {
        float totalItems = (float)N;
        float ratio      = (float)visRows / totalItems;
        float sbH        = (float)h - (listY0-(float)y);
        float thumbH     = std::max(16.f, sbH*ratio);
        float thumbY     = listY0 + (float)m_scroll/(float)maxScr*(sbH-thumbH);
        SkRRect sr; sr.setRectXY(SkRect::MakeXYWH((float)(x+w-kSbW+1.f),thumbY+1.f,(float)(kSbW-2.f),thumbH-2.f),3.f,3.f);
        SkPaint sbP; sbP.setAntiAlias(true); sbP.setColor(th.inputBorder);
        canvas->drawRRect(sr,sbP);
    }

    // Outer border
    SkPaint brd; brd.setAntiAlias(true); brd.setStyle(SkPaint::kStroke_Style);
    brd.setStrokeWidth(0.75f); brd.setColor(th.panelBorder);
    canvas->drawRect(SkRect::MakeXYWH((float)x,(float)y,(float)w,(float)h),brd);

    canvas->restore();
}

void skInspector::OnEvent(const skEvent& ev) {
    if (ev.type == skEventType::MouseMove) {
        if (!contains(ev.x,ev.y)) { m_hover=-1; return; }
        float listY0 = (float)(y+4+kPad+22);
        int idx = (int)((float)(ev.y-(int)listY0)/(float)kRowH) + m_scroll;
        m_hover = (ev.y >= (int)listY0) ? idx : -1;
    }
    if (ev.type == skEventType::MouseWheel && contains(ev.x,ev.y)) {
        const auto& th = skGetTheme();
        (void)th;
        constexpr int N = 12;
        float listY0 = (float)(y+4+kPad+22);
        int visRows = std::max(1, (int)(((float)h-(listY0-(float)y))/(float)kRowH));
        int maxScr  = std::max(0, N - visRows);
        if (ev.button > 0) m_scroll = std::max(0,       m_scroll - 1);
        else               m_scroll = std::min(maxScr,  m_scroll + 1);
    }
}
