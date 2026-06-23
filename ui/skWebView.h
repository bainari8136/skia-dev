#pragma once
#include "skWidget.h"
#include <string>
#include <functional>
#include <memory>

// WebView2-backed browser with a Skia-drawn URL/navigation bar.
// setOnNavigate() is called when the browser's source changes.
class skWebView : public skWidget {
public:
    skWebView(int x, int y, int w, int h);
    ~skWebView() override;

    void navigate(const std::string& url);
    void back();
    void forward();
    void refresh();

    const std::string& currentUrl() const { return m_url; }

    void setOnNavigate(std::function<void(const std::string&)> fn) { m_onNavigate = std::move(fn); }

    void Paint(SkCanvas* canvas) override;
    void OnEvent(const skEvent& ev) override;
    void setNativeHost(void* host) override;
    void syncNativeView(bool visible) override;
    bool canFocus() const override { return true; }

private:
    std::string m_url   = "about:blank";
    std::string m_editBuf;
    bool        m_urlFocused = false;
    bool        m_backHov = false, m_fwdHov = false, m_goHov = false;

    bool m_canGoBack = false;
    bool m_canGoForward = false;

    std::function<void(const std::string&)> m_onNavigate;

    struct Impl;
    std::shared_ptr<Impl> m_impl;

    void ensureInitialized();
    void updateNavigationState();
    void sourceChanged(const std::string& url);

    static constexpr int kBarH  = 32;
    static constexpr int kBtnW  = 28;

    SkRect barRect()  const;
    SkRect backBtn()  const;
    SkRect fwdBtn()   const;
    SkRect urlRect()  const;
    SkRect goBtn()    const;
    SkRect bodyRect() const;
};
