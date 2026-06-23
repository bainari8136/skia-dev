#include <Windows.h>
#include <objbase.h>
#include <WebView2.h>
#include <wrl.h>
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
#include <filesystem>
#include <sstream>

using Microsoft::WRL::Callback;
using Microsoft::WRL::ComPtr;

static sk_sp<SkTypeface>& pvTf() {
    static sk_sp<SkTypeface> tf = skGetSystemTypeface();
    return tf;
}

static std::wstring pdfToWide(const std::string& value) {
    if (value.empty()) return {};
    int count = MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, value.data(),
                                    static_cast<int>(value.size()), nullptr, 0);
    if (count <= 0) return {};
    std::wstring result(static_cast<size_t>(count), L'\0');
    MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, value.data(),
                        static_cast<int>(value.size()), result.data(), count);
    return result;
}

static std::string percentEncodePath(std::string value) {
    std::ostringstream encoded;
    encoded << std::uppercase << std::hex;
    for (unsigned char ch : value) {
        const bool safe = (ch >= 'a' && ch <= 'z') || (ch >= 'A' && ch <= 'Z') ||
                          (ch >= '0' && ch <= '9') || ch == '-' || ch == '_' ||
                          ch == '.' || ch == '~' || ch == '/' || ch == ':';
        if (safe) encoded << static_cast<char>(ch);
        else encoded << '%' << static_cast<int>(ch >> 4) << static_cast<int>(ch & 0x0f);
    }
    return encoded.str();
}

static std::string fileUrl(const std::string& path) {
    if (path.empty()) return {};
    std::error_code ec;
    auto absolute = std::filesystem::absolute(std::filesystem::u8path(path), ec);
    std::string normalized = ec ? path : absolute.u8string();
    std::replace(normalized.begin(), normalized.end(), '\\', '/');
    return "file:///" + percentEncodePath(normalized);
}

struct skPdfView::Impl {
    skPdfView* owner = nullptr;
    HWND host = nullptr;
    bool initializationStarted = false;
    bool requestedVisible = false;
    HRESULT initializationError = S_OK;
    ComPtr<ICoreWebView2Controller> controller;
    ComPtr<ICoreWebView2> webview;
};

skPdfView::skPdfView(int sx, int sy, int sw, int sh)
    : skWidget(sx, sy, sw, sh), m_impl(std::make_shared<Impl>()) {
    m_impl->owner = this;
}

skPdfView::~skPdfView() {
    m_impl->owner = nullptr;
    if (m_impl->controller) m_impl->controller->Close();
}

SkRect skPdfView::toolbar()  const { return SkRect::MakeXYWH((float)x,(float)y,(float)w,(float)kBarH); }
SkRect skPdfView::prevBtn()  const { return SkRect::MakeXYWH((float)(x+4),(float)(y+3),28.f,kBarH-6.f); }
SkRect skPdfView::nextBtn()  const { SkRect p=prevBtn(); return SkRect::MakeXYWH(p.right()+4.f,p.top(),28.f,p.height()); }
SkRect skPdfView::pageArea() const { return SkRect::MakeXYWH((float)x,(float)(y+kBarH),(float)w,(float)std::max(0,h-kBarH)); }

void skPdfView::setNativeHost(void* host) {
    m_impl->host = static_cast<HWND>(host);
    ensureInitialized();
}

void skPdfView::ensureInitialized() {
    auto state = m_impl;
    if (!state->host || state->initializationStarted) return;
    state->initializationStarted = true;

    HRESULT hr = CreateCoreWebView2EnvironmentWithOptions(
        nullptr, nullptr, nullptr,
        Callback<ICoreWebView2CreateCoreWebView2EnvironmentCompletedHandler>(
            [state](HRESULT result, ICoreWebView2Environment* environment) -> HRESULT {
                if (FAILED(result) || !environment) {
                    state->initializationError = FAILED(result) ? result : E_FAIL;
                    if (state->host) InvalidateRect(state->host, nullptr, FALSE);
                    return S_OK;
                }
                return environment->CreateCoreWebView2Controller(
                    state->host,
                    Callback<ICoreWebView2CreateCoreWebView2ControllerCompletedHandler>(
                        [state](HRESULT controllerResult, ICoreWebView2Controller* controller) -> HRESULT {
                            if (FAILED(controllerResult) || !controller) {
                                state->initializationError = FAILED(controllerResult) ? controllerResult : E_FAIL;
                                if (state->host) InvalidateRect(state->host, nullptr, FALSE);
                                return S_OK;
                            }
                            state->controller = controller;
                            controller->get_CoreWebView2(&state->webview);
                            if (!state->webview) {
                                state->initializationError = E_NOINTERFACE;
                                return S_OK;
                            }
                            if (state->owner) {
                                state->owner->syncNativeView(state->requestedVisible);
                                state->owner->navigateDocument();
                            }
                            if (state->host) InvalidateRect(state->host, nullptr, FALSE);
                            return S_OK;
                        }).Get());
            }).Get());
    if (FAILED(hr)) {
        state->initializationError = hr;
        if (state->host) InvalidateRect(state->host, nullptr, FALSE);
    }
}

void skPdfView::syncNativeView(bool visible) {
    m_impl->requestedVisible = visible;
    ensureInitialized();
    if (!m_impl->controller) return;
    RECT bounds { x, y + kBarH, x + std::max(0, w), y + std::max(kBarH, h) };
    m_impl->controller->put_Bounds(bounds);
    m_impl->controller->put_IsVisible(visible && !m_filepath.empty() && w > 0 && h > kBarH);
}

void skPdfView::loadFile(const std::string& path) {
    m_filepath = path;
    m_filename = path;
    auto pos = path.find_last_of("/\\");
    if (pos != std::string::npos) m_filename = path.substr(pos+1);
    if (m_pageCount == 0) m_pageCount = 1;
    m_page = 1;
    navigateDocument();
}

void skPdfView::navigateDocument() {
    if (!m_impl->webview || m_filepath.empty()) return;
    std::string url = fileUrl(m_filepath);
    url += "#page=" + std::to_string(std::max(1, m_page));
    url += "&zoom=" + std::to_string(static_cast<int>(m_zoom * 100.f));
    const std::wstring wide = pdfToWide(url);
    if (!wide.empty()) m_impl->webview->Navigate(wide.c_str());
}

void skPdfView::goToPage(int page) {
    if (m_pageCount <= 0) return;
    const int clamped = std::max(1, std::min(m_pageCount, page));
    if (clamped == m_page) return;
    m_page = clamped;
    navigateDocument();
    if (m_onPageChange) m_onPageChange(m_page);
}

void skPdfView::setZoom(float z) {
    const float clamped = std::max(0.25f, std::min(4.f, z));
    if (clamped == m_zoom) return;
    m_zoom = clamped;
    navigateDocument();
}

void skPdfView::Paint(SkCanvas* canvas) {
    const auto& th = skGetTheme();
    SkFont font(pvTf(), 11.f); font.setEdging(SkFont::Edging::kAntiAlias);
    SkFont smallFont(pvTf(), 10.f); smallFont.setEdging(SkFont::Edging::kAntiAlias);

    SkRRect outerRR; outerRR.setRectXY(SkRect::MakeXYWH((float)x,(float)y,(float)w,(float)h),6.f,6.f);
    canvas->save(); canvas->clipRRect(outerRR, SkClipOp::kIntersect, true);
    SkPaint bgP; bgP.setAntiAlias(true); bgP.setColor(th.panelBg); canvas->drawRRect(outerRR,bgP);

    SkPaint barP; barP.setColor(th.inputBg); canvas->drawRect(toolbar(),barP);
    auto drawNavBtn = [&](SkRect r, const char* sym, bool hov, bool enabled) {
        SkRRect br; br.setRectXY(r,3.f,3.f);
        if (hov && enabled) {
            SkPaint hp; hp.setAntiAlias(true); hp.setColor(SkColorSetARGB(40,128,128,128));
            canvas->drawRRect(br,hp);
        }
        SkPaint tp; tp.setAntiAlias(true); tp.setColor(enabled ? th.textPrimary : th.textSecondary);
        SkRect tb; font.measureText(sym,strlen(sym),SkTextEncoding::kUTF8,&tb);
        canvas->drawString(sym,r.centerX()-tb.width()/2.f-tb.left(),r.centerY()-tb.height()/2.f-tb.top(),font,tp);
    };
    drawNavBtn(prevBtn(), "\xe2\x97\x80", m_prevHov, m_page > 1);
    drawNavBtn(nextBtn(), "\xe2\x96\xb6", m_nextHov, m_page > 0 && m_page < m_pageCount);

    char info[96];
    if (m_pageCount > 0)
        snprintf(info,sizeof(info),"Page %d / %d    Zoom: %.0f%%",m_page,m_pageCount,m_zoom*100.f);
    else
        snprintf(info,sizeof(info),"No document loaded");
    SkPaint ip; ip.setAntiAlias(true); ip.setColor(th.textSecondary);
    canvas->drawString(info, nextBtn().right()+10.f, toolbar().centerY()+4.f, smallFont, ip);

    SkPaint dp; dp.setColor(th.panelBorder);
    canvas->drawLine((float)x,(float)(y+kBarH),(float)(x+w),(float)(y+kBarH),dp);

    SkRect pa = pageArea();
    SkPaint pageBg; pageBg.setColor(SkColorSetRGB(200,200,205)); canvas->drawRect(pa, pageBg);
    if (m_filepath.empty() || !m_impl->webview) {
        SkPaint status; status.setAntiAlias(true);
        status.setColor(FAILED(m_impl->initializationError) ? SK_ColorRED : SkColorSetRGB(100,110,130));
        const char* message = m_filepath.empty() ? "No PDF loaded" :
            (FAILED(m_impl->initializationError)
                ? "PDF renderer unavailable. Install the WebView2 Runtime."
                : "Initializing PDF renderer...");
        canvas->drawString(message, pa.left()+12.f, pa.top()+26.f, smallFont, status);
    }

    SkPaint brd; brd.setAntiAlias(true); brd.setStyle(SkPaint::kStroke_Style);
    brd.setStrokeWidth(0.75f); brd.setColor(th.panelBorder); canvas->drawRRect(outerRR,brd);
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
