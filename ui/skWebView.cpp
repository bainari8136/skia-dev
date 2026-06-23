#include <Windows.h>
#include <objbase.h>
#include <WebView2.h>
#include <wrl.h>
#include "skWebView.h"
#include "skTheme.h"
#include "skTypeface.h"
#include <include/core/SkPaint.h>
#include <include/core/SkFont.h>
#include <include/core/SkCanvas.h>
#include <include/core/SkRRect.h>
#include <algorithm>
#include <cstring>

using Microsoft::WRL::Callback;
using Microsoft::WRL::ComPtr;

static sk_sp<SkTypeface>& wvTf() {
    static sk_sp<SkTypeface> tf = skGetSystemTypeface();
    return tf;
}

static std::wstring toWide(const std::string& value) {
    if (value.empty()) return {};
    int count = MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, value.data(),
                                    static_cast<int>(value.size()), nullptr, 0);
    if (count <= 0) return {};
    std::wstring result(static_cast<size_t>(count), L'\0');
    MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, value.data(),
                        static_cast<int>(value.size()), result.data(), count);
    return result;
}

static std::string toUtf8(const wchar_t* value) {
    if (!value || !*value) return {};
    const int inputLength = static_cast<int>(wcslen(value));
    int count = WideCharToMultiByte(CP_UTF8, 0, value, inputLength, nullptr, 0, nullptr, nullptr);
    if (count <= 0) return {};
    std::string result(static_cast<size_t>(count), '\0');
    WideCharToMultiByte(CP_UTF8, 0, value, inputLength, result.data(), count, nullptr, nullptr);
    return result;
}

static std::string normalizedUrl(std::string url) {
    const auto first = url.find_first_not_of(" \t\r\n");
    const auto last = url.find_last_not_of(" \t\r\n");
    if (first == std::string::npos) return "about:blank";
    url = url.substr(first, last - first + 1);
    if (url.find("://") == std::string::npos && url.rfind("about:", 0) != 0 &&
        url.rfind("data:", 0) != 0 && url.rfind("file:", 0) != 0) {
        url.insert(0, "https://");
    }
    return url;
}

struct skWebView::Impl {
    skWebView* owner = nullptr;
    HWND host = nullptr;
    bool initializationStarted = false;
    bool requestedVisible = false;
    HRESULT initializationError = S_OK;
    EventRegistrationToken sourceChangedToken {};
    EventRegistrationToken historyChangedToken {};
    ComPtr<ICoreWebView2Controller> controller;
    ComPtr<ICoreWebView2> webview;
};

skWebView::skWebView(int sx, int sy, int sw, int sh)
    : skWidget(sx, sy, sw, sh), m_impl(std::make_shared<Impl>()) {
    m_impl->owner = this;
    m_editBuf = m_url;
}

skWebView::~skWebView() {
    m_impl->owner = nullptr;
    if (m_impl->controller) m_impl->controller->Close();
}

SkRect skWebView::barRect()  const { return SkRect::MakeXYWH((float)x,(float)y,(float)w,(float)kBarH); }
SkRect skWebView::backBtn()  const { return SkRect::MakeXYWH((float)(x+4),(float)(y+3),(float)kBtnW,kBarH-6.f); }
SkRect skWebView::fwdBtn()   const { SkRect b=backBtn(); return SkRect::MakeXYWH(b.right()+2.f,b.top(),(float)kBtnW,b.height()); }
SkRect skWebView::goBtn()    const { return SkRect::MakeXYWH((float)(x+w-kBtnW-4),(float)(y+3),(float)kBtnW,kBarH-6.f); }
SkRect skWebView::urlRect()  const { SkRect f=fwdBtn(),g=goBtn(); return SkRect::MakeXYWH(f.right()+4.f,(float)(y+3),std::max(0.f,g.left()-f.right()-8.f),kBarH-6.f); }
SkRect skWebView::bodyRect() const { return SkRect::MakeXYWH((float)x,(float)(y+kBarH),(float)w,(float)std::max(0,h-kBarH)); }

void skWebView::setNativeHost(void* host) {
    m_impl->host = static_cast<HWND>(host);
    ensureInitialized();
}

void skWebView::ensureInitialized() {
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

                            state->webview->add_SourceChanged(
                                Callback<ICoreWebView2SourceChangedEventHandler>(
                                    [state](ICoreWebView2* sender, ICoreWebView2SourceChangedEventArgs*) -> HRESULT {
                                        LPWSTR source = nullptr;
                                        if (SUCCEEDED(sender->get_Source(&source)) && source) {
                                            if (state->owner) state->owner->sourceChanged(toUtf8(source));
                                            CoTaskMemFree(source);
                                        }
                                        return S_OK;
                                    }).Get(), &state->sourceChangedToken);
                            state->webview->add_HistoryChanged(
                                Callback<ICoreWebView2HistoryChangedEventHandler>(
                                    [state](ICoreWebView2*, IUnknown*) -> HRESULT {
                                        if (state->owner) state->owner->updateNavigationState();
                                        return S_OK;
                                    }).Get(), &state->historyChangedToken);

                            if (state->owner) {
                                state->owner->syncNativeView(state->requestedVisible);
                                state->owner->navigate(state->owner->m_url);
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

void skWebView::syncNativeView(bool visible) {
    m_impl->requestedVisible = visible;
    ensureInitialized();
    if (!m_impl->controller) return;
    RECT bounds { x, y + kBarH, x + std::max(0, w), y + std::max(kBarH, h) };
    m_impl->controller->put_Bounds(bounds);
    m_impl->controller->put_IsVisible(visible && w > 0 && h > kBarH);
}

void skWebView::navigate(const std::string& url) {
    m_url = normalizedUrl(url);
    m_editBuf = m_url;
    m_urlFocused = false;
    if (m_impl->webview) {
        const std::wstring wide = toWide(m_url);
        if (!wide.empty()) m_impl->webview->Navigate(wide.c_str());
    }
}

void skWebView::back() {
    if (m_impl->webview && m_canGoBack) m_impl->webview->GoBack();
}

void skWebView::forward() {
    if (m_impl->webview && m_canGoForward) m_impl->webview->GoForward();
}

void skWebView::refresh() {
    if (m_impl->webview) m_impl->webview->Reload();
}

void skWebView::updateNavigationState() {
    BOOL value = FALSE;
    m_canGoBack = SUCCEEDED(m_impl->webview->get_CanGoBack(&value)) && value;
    value = FALSE;
    m_canGoForward = SUCCEEDED(m_impl->webview->get_CanGoForward(&value)) && value;
    if (m_impl->host) InvalidateRect(m_impl->host, nullptr, FALSE);
}

void skWebView::sourceChanged(const std::string& url) {
    m_url = url;
    if (!m_urlFocused) m_editBuf = url;
    updateNavigationState();
    if (m_onNavigate) m_onNavigate(url);
}

void skWebView::Paint(SkCanvas* canvas) {
    const auto& th = skGetTheme();
    SkFont font(wvTf(), 11.f); font.setEdging(SkFont::Edging::kAntiAlias);
    SkFont smallFont(wvTf(), 10.f); smallFont.setEdging(SkFont::Edging::kAntiAlias);

    SkRRect rr; rr.setRectXY(SkRect::MakeXYWH((float)x,(float)y,(float)w,(float)h),6.f,6.f);
    canvas->save(); canvas->clipRRect(rr, SkClipOp::kIntersect, true);
    SkPaint bgP; bgP.setColor(th.panelBg); canvas->drawRRect(rr,bgP);

    SkPaint barP; barP.setColor(th.inputBg); canvas->drawRect(barRect(),barP);
    auto drawNavBtn = [&](SkRect r, const char* sym, bool hov, bool enabled) {
        if (hov && enabled) {
            SkPaint hp; hp.setAntiAlias(true); hp.setColor(SkColorSetARGB(40,128,128,128));
            SkRRect br; br.setRectXY(r,3.f,3.f); canvas->drawRRect(br,hp);
        }
        SkPaint tp; tp.setAntiAlias(true); tp.setColor(enabled ? th.textPrimary : th.textSecondary);
        SkRect tb; font.measureText(sym,strlen(sym),SkTextEncoding::kUTF8,&tb);
        canvas->drawString(sym,r.centerX()-tb.width()/2.f-tb.left(),r.centerY()-tb.height()/2.f-tb.top(),font,tp);
    };
    drawNavBtn(backBtn(), "\xe2\x86\x90", m_backHov, m_canGoBack);
    drawNavBtn(fwdBtn(),  "\xe2\x86\x92", m_fwdHov, m_canGoForward);

    SkRect ur = urlRect();
    SkPaint urlBg; urlBg.setColor(th.inputBg);
    SkPaint urlBrd; urlBrd.setAntiAlias(true); urlBrd.setStyle(SkPaint::kStroke_Style);
    urlBrd.setStrokeWidth(1.f); urlBrd.setColor(m_urlFocused ? th.accent : th.inputBorder);
    SkRRect urr; urr.setRectXY(ur,3.f,3.f);
    canvas->drawRRect(urr,urlBg); canvas->drawRRect(urr,urlBrd);
    canvas->save(); canvas->clipRect(ur);
    SkPaint urlTp; urlTp.setAntiAlias(true); urlTp.setColor(th.textPrimary);
    const std::string& shownUrl = m_urlFocused ? m_editBuf : m_url;
    SkRect textBounds; smallFont.measureText(shownUrl.c_str(), shownUrl.size(), SkTextEncoding::kUTF8, &textBounds);
    float textX = ur.left()+6.f;
    if (m_urlFocused && textBounds.width() > ur.width()-12.f)
        textX -= textBounds.width() - (ur.width()-12.f);
    canvas->drawString(shownUrl.c_str(), textX, ur.centerY()+4.f, smallFont, urlTp);
    if (m_urlFocused) {
        float cx = textX + textBounds.width();
        SkPaint cp; cp.setColor(th.accent);
        canvas->drawLine(cx,ur.top()+4.f,cx,ur.bottom()-4.f,cp);
    }
    canvas->restore();
    drawNavBtn(goBtn(), "\xe2\x86\x92", m_goHov, true);

    SkRect body = bodyRect();
    SkPaint bodyBg; bodyBg.setColor(SkColorSetRGB(250,250,252)); canvas->drawRect(body, bodyBg);
    if (!m_impl->webview) {
        SkPaint status; status.setAntiAlias(true);
        status.setColor(FAILED(m_impl->initializationError) ? SK_ColorRED : SkColorSetRGB(100,120,160));
        const char* message = FAILED(m_impl->initializationError)
            ? "WebView2 could not be initialized. Install the WebView2 Runtime."
            : "Initializing WebView2...";
        canvas->drawString(message, body.left()+12.f, body.top()+26.f, smallFont, status);
    }

    SkPaint divP; divP.setColor(th.panelBorder);
    canvas->drawLine((float)x,(float)(y+kBarH),(float)(x+w),(float)(y+kBarH),divP);
    SkPaint brd; brd.setAntiAlias(true); brd.setStyle(SkPaint::kStroke_Style);
    brd.setStrokeWidth(0.75f); brd.setColor(th.panelBorder); canvas->drawRRect(rr,brd);
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
    if (ev.type == skEventType::KeyChar && m_urlFocused && ev.ch >= 32) {
        wchar_t chars[2] = { ev.ch, 0 };
        m_editBuf += toUtf8(chars);
    }
    if (ev.type == skEventType::KeyDown && m_urlFocused) {
        if (ev.button == VK_BACK && !m_editBuf.empty()) {
            do { m_editBuf.pop_back(); }
            while (!m_editBuf.empty() && (static_cast<unsigned char>(m_editBuf.back()) & 0xC0) == 0x80);
        }
        if (ev.button == VK_RETURN) navigate(m_editBuf);
        if (ev.button == VK_ESCAPE) { m_urlFocused = false; m_editBuf = m_url; }
    }
}
