#pragma once
#include "skWidget.h"
#include <string>
#include <vector>
#include <functional>

// Web-browser placeholder widget. Displays a URL bar with navigation buttons
// and a content area. Actual HTML rendering is not implemented — the content
// area shows a placeholder. Wire setOnNavigate() to handle URL changes.
class skWebView : public skWidget {
public:
    skWebView(int x, int y, int w, int h);

    void navigate(const std::string& url);
    void back();
    void forward();
    void refresh();

    const std::string& currentUrl() const { return m_url; }

    void setOnNavigate(std::function<void(const std::string&)> fn) { m_onNavigate = std::move(fn); }

    void Paint(SkCanvas* canvas) override;
    void OnEvent(const skEvent& ev) override;
    bool canFocus() const override { return true; }

private:
    std::string m_url   = "about:blank";
    std::string m_editBuf;
    bool        m_urlFocused = false;
    bool        m_backHov = false, m_fwdHov = false, m_goHov = false;

    std::vector<std::string> m_history;
    int                      m_histIdx = -1;

    std::function<void(const std::string&)> m_onNavigate;

    static constexpr int kBarH  = 32;
    static constexpr int kBtnW  = 28;

    SkRect barRect()  const;
    SkRect backBtn()  const;
    SkRect fwdBtn()   const;
    SkRect urlRect()  const;
    SkRect goBtn()    const;
    SkRect bodyRect() const;
};
