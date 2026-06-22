#include <Windows.h>
#include "skCodeEditor.h"
#include "skTheme.h"
#include "skTypeface.h"
#include <include/core/SkPaint.h>
#include <include/core/SkFont.h>
#include <include/core/SkRRect.h>
#include <fstream>
#include <sstream>
#include <unordered_set>
#include <cctype>
#include <algorithm>
#include <cstdio>

static sk_sp<SkTypeface>& ceTf() {
    static sk_sp<SkTypeface> tf = skGetSystemTypeface();
    return tf;
}

static const SkColor kColDefault = 0xFFD4D4D4;
static const SkColor kColKeyword = 0xFF569CD6;
static const SkColor kColType    = 0xFF4EC9B0;
static const SkColor kColString  = 0xFFCE9178;
static const SkColor kColComment = 0xFF6A9955;
static const SkColor kColPrepro  = 0xFFC586C0;
static const SkColor kColNumber  = 0xFFB5CEA8;
static const SkColor kColGutter  = 0xFF858585;

static const std::unordered_set<std::string>& cppKeywords() {
    static const std::unordered_set<std::string> kws = {
        "auto","break","case","catch","class","const","constexpr","continue",
        "default","delete","do","double","else","enum","explicit","extern",
        "false","float","for","friend","goto","if","inline","int","long",
        "namespace","new","noexcept","nullptr","operator","override","private",
        "protected","public","return","short","signed","sizeof","static",
        "struct","switch","template","this","throw","true","try","typedef",
        "typename","union","unsigned","using","virtual","void","volatile","while",
        "bool","char","wchar_t","size_t","int8_t","int16_t","int32_t","int64_t",
        "uint8_t","uint16_t","uint32_t","uint64_t","nullptr_t","decltype",
        "static_assert","static_cast","reinterpret_cast","const_cast","dynamic_cast"
    };
    return kws;
}

static const std::unordered_set<std::string>& skiaTypes() {
    static const std::unordered_set<std::string> ts = {
        "sk_sp","SkColor","SkCanvas","SkPaint","SkFont","SkRect","SkRRect",
        "SkPoint","SkPath","SkMatrix","SkSurface","SkBitmap","SkPixmap",
        "SkTypeface","SkShader","skWidget","skButton","skLabel","skWindow",
        "skApp","skEvent","skSizer","skTheme","HWND","HINSTANCE","WPARAM",
        "LPARAM","LRESULT","HANDLE","DWORD","BOOL","std","string","vector",
        "shared_ptr","unique_ptr","function","move"
    };
    return ts;
}

skCodeEditor::skCodeEditor(int sx, int sy, int sw, int sh)
    : skWidget(sx, sy, sw, sh) {}

void skCodeEditor::setText(const std::string& code) {
    m_lines.clear();
    m_scrollY = m_scrollX = 0;
    std::istringstream ss(code);
    std::string line;
    while (std::getline(ss, line)) {
        // Strip \r
        if (!line.empty() && line.back() == '\r') line.pop_back();
        m_lines.push_back(std::move(line));
    }
}

void skCodeEditor::loadFile(const std::string& path) {
    std::ifstream f(path);
    if (!f.is_open()) return;
    std::ostringstream ss;
    ss << f.rdbuf();
    setText(ss.str());
}

void skCodeEditor::tokenizeLine(const std::string& line, std::vector<Token>& out) const {
    size_t n = line.size(), i = 0;

    // Preprocessor
    size_t first = line.find_first_not_of(" \t");
    if (first != std::string::npos && line[first] == '#') {
        out.push_back({line, kColPrepro});
        return;
    }

    while (i < n) {
        // Line comment
        if (i + 1 < n && line[i] == '/' && line[i+1] == '/') {
            out.push_back({line.substr(i), kColComment});
            return;
        }
        // Block comment start
        if (i + 1 < n && line[i] == '/' && line[i+1] == '*') {
            size_t end = line.find("*/", i+2);
            if (end == std::string::npos) end = n - 2;
            out.push_back({line.substr(i, end+2-i), kColComment});
            i = std::min(n, end + 2);
            continue;
        }
        // String literal
        if (line[i] == '"') {
            size_t j = i + 1;
            while (j < n && !(line[j] == '"' && line[j-1] != '\\')) ++j;
            if (j < n) ++j;
            out.push_back({line.substr(i, j-i), kColString});
            i = j;
            continue;
        }
        // Char literal
        if (line[i] == '\'') {
            size_t j = i + 1;
            while (j < n && line[j] != '\'' && j - i < 8) ++j;
            if (j < n) ++j;
            out.push_back({line.substr(i, j-i), kColString});
            i = j;
            continue;
        }
        // Number (including hex 0x…)
        if (std::isdigit((unsigned char)line[i])) {
            size_t j = i;
            bool hex = (j+1 < n && line[j]=='0' && (line[j+1]=='x' || line[j+1]=='X'));
            while (j < n && (std::isalnum((unsigned char)line[j]) || line[j]=='.')) ++j;
            out.push_back({line.substr(i, j-i), kColNumber});
            i = j;
            continue;
        }
        // Identifier
        if (std::isalpha((unsigned char)line[i]) || line[i] == '_') {
            size_t j = i;
            while (j < n && (std::isalnum((unsigned char)line[j]) || line[j]=='_')) ++j;
            std::string word = line.substr(i, j-i);
            SkColor c = kColDefault;
            if (cppKeywords().count(word)) c = kColKeyword;
            else if (skiaTypes().count(word)) c = kColType;
            out.push_back({word, c});
            i = j;
            continue;
        }
        // Punctuation / spaces (collect run)
        size_t j = i + 1;
        while (j < n && !std::isalnum((unsigned char)line[j]) && line[j]!='_'
               && line[j]!='"' && line[j]!='\''
               && !(j+1 < n && line[j]=='/' && (line[j+1]=='/' || line[j+1]=='*'))) {
            ++j;
        }
        out.push_back({line.substr(i, j-i), kColDefault});
        i = j;
    }
}

void skCodeEditor::Paint(SkCanvas* canvas) {
    SkFont font(ceTf(), kFontSz); font.setEdging(SkFont::Edging::kAntiAlias);

    float fx = (float)x, fy = (float)y, fw = (float)w, fh = (float)h;

    // Dark background
    SkRRect rr; rr.setRectXY(SkRect::MakeXYWH(fx,fy,fw,fh), 4.f, 4.f);
    SkPaint bgP; bgP.setAntiAlias(true); bgP.setColor(SkColorSetRGB(30, 30, 30));
    canvas->drawRRect(rr, bgP);

    // Line number gutter
    SkPaint gutterP; gutterP.setColor(SkColorSetRGB(37, 37, 38));
    canvas->save();
    canvas->clipRRect(rr, SkClipOp::kIntersect, true);
    canvas->drawRect(SkRect::MakeXYWH(fx, fy, kGutterW, fh), gutterP);

    // Gutter separator
    SkPaint gsep; gsep.setColor(SkColorSetRGB(55,55,55));
    canvas->drawRect(SkRect::MakeXYWH(fx+kGutterW, fy, 1.f, fh), gsep);

    // Content area clip
    float codeX = fx + kGutterW + 2.f;
    float codeW  = fw - kGutterW - 2.f - kSbW;

    int vis = visibleLines();
    int end = std::min((int)m_lines.size(), m_scrollY + vis);

    std::vector<Token> tokens;
    for (int ln = m_scrollY; ln < end; ++ln) {
        float ly = fy + (float)(ln - m_scrollY) * kLineH + kLineH - 3.f;

        // Line number
        {
            char num[8]; snprintf(num, sizeof(num), "%d", ln+1);
            SkRect nb; font.measureText(num, strlen(num), SkTextEncoding::kUTF8, &nb);
            SkPaint np; np.setAntiAlias(true); np.setColor(kColGutter);
            canvas->drawString(num, fx + kGutterW - nb.width() - nb.left() - 4.f, ly, font, np);
        }

        // Tokenize and draw code
        tokens.clear();
        tokenizeLine(m_lines[ln], tokens);

        canvas->save();
        canvas->clipRect(SkRect::MakeXYWH(codeX, fy, codeW, fh));

        float tx = codeX - (float)m_scrollX;
        for (auto& tok : tokens) {
            if (tok.text.empty()) continue;
            SkRect tb; font.measureText(tok.text.c_str(), tok.text.size(),
                                         SkTextEncoding::kUTF8, &tb);
            SkPaint tp; tp.setAntiAlias(true); tp.setColor(tok.color);
            canvas->drawString(tok.text.c_str(), tx - tb.left(), ly, font, tp);
            tx += tb.width();
        }
        canvas->restore();
    }

    canvas->restore();

    // Vertical scrollbar
    int ms = maxScrollY();
    if (ms > 0) {
        float sbX = fx + fw - kSbW;
        SkPaint trk; trk.setColor(SkColorSetRGB(37,37,38));
        canvas->drawRect(SkRect::MakeXYWH(sbX,fy,kSbW,fh-kSbH), trk);
        float ratio  = (float)vis / (float)(vis + ms);
        float thumbH = std::max(20.f, (fh - kSbH) * ratio);
        float thumbY = fy + (float)m_scrollY/(float)ms * (fh - kSbH - thumbH);
        SkRRect tRR; tRR.setRectXY(SkRect::MakeXYWH(sbX+1.f,thumbY+1.f,kSbW-2.f,thumbH-2.f),3.f,3.f);
        SkPaint tP; tP.setAntiAlias(true); tP.setColor(SkColorSetRGB(80,84,96));
        canvas->drawRRect(tRR, tP);
    }

    // Outer border
    SkPaint brd; brd.setAntiAlias(true);
    brd.setStyle(SkPaint::kStroke_Style); brd.setStrokeWidth(1.f);
    brd.setColor(SkColorSetRGB(55,55,55));
    canvas->drawRRect(rr, brd);
}

void skCodeEditor::OnEvent(const skEvent& ev) {
    if (!contains(ev.x, ev.y)) return;
    if (ev.type == skEventType::MouseWheel) {
        int ms = maxScrollY();
        if (ev.button > 0) m_scrollY = std::max(0,  m_scrollY - 3);
        else               m_scrollY = std::min(ms, m_scrollY + 3);
    }
}
