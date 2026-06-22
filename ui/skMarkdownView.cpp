#include "skMarkdownView.h"
#include "skTheme.h"
#include "skTypeface.h"
#include <include/core/SkPaint.h>
#include <include/core/SkFont.h>
#include <include/core/SkCanvas.h>
#include <include/core/SkRRect.h>
#include <sstream>
#include <algorithm>

static sk_sp<SkTypeface>& mdTf() {
    static sk_sp<SkTypeface> tf = skGetSystemTypeface();
    return tf;
}
static sk_sp<SkTypeface>& mdMono() {
    static sk_sp<SkTypeface> tf = skGetSystemTypeface();
    return tf;
}

skMarkdownView::skMarkdownView(int sx, int sy, int sw, int sh)
    : skWidget(sx, sy, sw, sh) {}

void skMarkdownView::setContent(const std::string& md) {
    m_raw = md;
    m_scroll = 0;
    parse();
}

void skMarkdownView::parse() {
    m_blocks.clear();
    std::istringstream ss(m_raw);
    std::string line;
    bool inCode = false;
    while (std::getline(ss, line)) {
        if (line.substr(0,3) == "```") { inCode = !inCode; continue; }
        if (inCode) { m_blocks.push_back({BlockType::CodeLine, line}); continue; }
        if (line == "---" || line == "***" || line == "___") {
            m_blocks.push_back({BlockType::Rule, ""}); continue;
        }
        if (line.substr(0,4) == "### ") { m_blocks.push_back({BlockType::H3, line.substr(4)}); continue; }
        if (line.substr(0,3) == "## ")  { m_blocks.push_back({BlockType::H2, line.substr(3)}); continue; }
        if (line.substr(0,2) == "# ")   { m_blocks.push_back({BlockType::H1, line.substr(2)}); continue; }
        if (line.size()>2 && (line[0]=='-'||line[0]=='*') && line[1]==' ')
            { m_blocks.push_back({BlockType::BulletItem, line.substr(2)}); continue; }
        if (!line.empty()) { m_blocks.push_back({BlockType::Para, line}); continue; }
    }
}

static std::vector<std::string> wordWrap(const std::string& text, const SkFont& font, float maxW) {
    std::vector<std::string> lines;
    std::istringstream ss(text);
    std::string word, cur;
    while (ss >> word) {
        std::string test = cur.empty() ? word : cur + " " + word;
        SkRect b; font.measureText(test.c_str(), test.size(), SkTextEncoding::kUTF8, &b);
        if (!cur.empty() && b.width() > maxW) { lines.push_back(cur); cur = word; }
        else cur = test;
    }
    if (!cur.empty()) lines.push_back(cur);
    if (lines.empty()) lines.push_back("");
    return lines;
}

void skMarkdownView::Paint(SkCanvas* canvas) {
    const auto& th = skGetTheme();
    SkPaint bgP; bgP.setColor(th.panelBg);
    canvas->drawRect(SkRect::MakeXYWH((float)x,(float)y,(float)w,(float)h), bgP);
    canvas->save();
    canvas->clipRect(SkRect::MakeXYWH((float)x,(float)y,(float)w,(float)h));

    float innerW = (float)(w - 2*kPad - kSbW);
    float curY   = (float)(y + kPad) - (float)m_scroll;

    SkFont h1Font(mdTf(), 22.f); h1Font.setEdging(SkFont::Edging::kAntiAlias);
    SkFont h2Font(mdTf(), 18.f); h2Font.setEdging(SkFont::Edging::kAntiAlias);
    SkFont h3Font(mdTf(), 15.f); h3Font.setEdging(SkFont::Edging::kAntiAlias);
    SkFont pFont (mdTf(), 13.f); pFont .setEdging(SkFont::Edging::kAntiAlias);
    SkFont monoF (mdMono() ? mdMono() : mdTf(), 12.f); monoF.setEdging(SkFont::Edging::kAntiAlias);

    SkPaint tp; tp.setAntiAlias(true); tp.setColor(th.textPrimary);
    SkPaint t2; t2.setAntiAlias(true); t2.setColor(th.textSecondary);
    SkPaint ac; ac.setAntiAlias(true); ac.setColor(th.accent);

    m_contentH = 0;
    for (auto& b : m_blocks) {
        switch (b.type) {
        case BlockType::H1: {
            SkRect meas; h1Font.measureText(b.text.c_str(),b.text.size(),SkTextEncoding::kUTF8,&meas);
            canvas->drawString(b.text.c_str(),(float)(x+kPad),curY-meas.top(),h1Font,tp);
            curY += meas.height()+10.f;
            // Underline
            SkPaint ul; ul.setColor(th.panelBorder); ul.setStrokeWidth(1.f);
            canvas->drawLine((float)(x+kPad),curY,(float)(x+kPad)+(innerW),curY,ul);
            curY += 10.f;
            break; }
        case BlockType::H2: {
            SkRect meas; h2Font.measureText(b.text.c_str(),b.text.size(),SkTextEncoding::kUTF8,&meas);
            canvas->drawString(b.text.c_str(),(float)(x+kPad),curY-meas.top(),h2Font,tp);
            curY += meas.height()+10.f; break; }
        case BlockType::H3: {
            SkRect meas; h3Font.measureText(b.text.c_str(),b.text.size(),SkTextEncoding::kUTF8,&meas);
            canvas->drawString(b.text.c_str(),(float)(x+kPad),curY-meas.top(),h3Font,t2);
            curY += meas.height()+8.f; break; }
        case BlockType::Para: {
            auto lines = wordWrap(b.text, pFont, innerW);
            SkRect m; pFont.measureText("X",1,SkTextEncoding::kUTF8,&m);
            float lh = m.height()+3.f;
            for (auto& l : lines) { canvas->drawString(l.c_str(),(float)(x+kPad),curY-m.top(),pFont,tp); curY+=lh; }
            curY += 6.f; break; }
        case BlockType::BulletItem: {
            auto lines = wordWrap(b.text, pFont, innerW-14.f);
            SkRect m; pFont.measureText("X",1,SkTextEncoding::kUTF8,&m);
            float lh = m.height()+3.f;
            SkPaint dot; dot.setAntiAlias(true); dot.setColor(th.accent);
            canvas->drawCircle((float)(x+kPad+4.f),curY-m.top()-m.height()*0.3f,2.5f,dot);
            for (int i=0;i<(int)lines.size();i++) {
                canvas->drawString(lines[i].c_str(),(float)(x+kPad+14.f),curY-m.top(),pFont,tp);
                curY+=lh;
            }
            curY+=4.f; break; }
        case BlockType::CodeLine: {
            SkRect m; monoF.measureText("X",1,SkTextEncoding::kUTF8,&m);
            float lh = m.height()+2.f;
            SkPaint codeBg; codeBg.setColor(SkColorSetARGB(30,128,128,128));
            canvas->drawRect(SkRect::MakeXYWH((float)(x+kPad),curY-2.f,innerW,lh+2.f),codeBg);
            SkPaint codeP; codeP.setAntiAlias(true); codeP.setColor(SkColorSetRGB(0xCE,0x91,0x78));
            canvas->drawString(b.text.c_str(),(float)(x+kPad+4.f),curY-m.top(),monoF,codeP);
            curY+=lh; break; }
        case BlockType::Rule: {
            SkPaint rl; rl.setColor(th.panelBorder); rl.setStrokeWidth(1.f);
            canvas->drawLine((float)(x+kPad),curY+4.f,(float)(x+kPad)+innerW,curY+4.f,rl);
            curY+=16.f; break; }
        }
    }
    m_contentH = (int)(curY - (float)(y+kPad) + (float)m_scroll + (float)kPad);

    // Scrollbar
    if (m_contentH > h) {
        float ratio = (float)h / (float)m_contentH;
        float thumbH = std::max(16.f, (float)h*ratio);
        float ms = (float)(m_contentH - h);
        float thumbY = (float)y + (float)m_scroll/ms*(float)(h-(int)thumbH);
        SkRRect sr; sr.setRectXY(SkRect::MakeXYWH((float)(x+w-kSbW+1.f),thumbY+1.f,(float)kSbW-2.f,thumbH-2.f),3.f,3.f);
        SkPaint sp; sp.setAntiAlias(true); sp.setColor(th.inputBorder);
        canvas->drawRRect(sr,sp);
    }
    canvas->restore();
}

void skMarkdownView::OnEvent(const skEvent& ev) {
    if (ev.type == skEventType::MouseWheel) {
        if (!contains(ev.x, ev.y)) return;
        int delta = (ev.button > 0) ? -30 : 30;
        int ms = std::max(0, m_contentH - h);
        m_scroll = std::max(0, std::min(ms, m_scroll + delta));
    }
}
