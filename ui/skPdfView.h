#pragma once
#include "skWidget.h"
#include <string>
#include <functional>
#include <memory>

// WebView2-backed PDF renderer with a Skia-drawn navigation toolbar.
// setOnPageChange() is called after API or toolbar page navigation.
class skPdfView : public skWidget {
public:
    skPdfView(int x, int y, int w, int h);
    ~skPdfView() override;

    void loadFile(const std::string& path);
    void goToPage(int page);          // 1-based
    void nextPage()  { goToPage(m_page+1); }
    void prevPage()  { goToPage(m_page-1); }
    void setZoom(float z);

    int         currentPage() const { return m_page; }
    int         pageCount()   const { return m_pageCount; }
    float       zoom()        const { return m_zoom; }
    std::string filename()    const { return m_filename; }

    void setPageCount(int n) { m_pageCount = n; }
    void setOnPageChange(std::function<void(int page)> fn) { m_onPageChange = std::move(fn); }

    void Paint(SkCanvas* canvas) override;
    void OnEvent(const skEvent& ev) override;
    void setNativeHost(void* host) override;
    void syncNativeView(bool visible) override;

private:
    std::string m_filepath;
    std::string m_filename;
    int         m_page      = 0;
    int         m_pageCount = 0;
    float       m_zoom      = 1.0f;
    bool        m_prevHov   = false;
    bool        m_nextHov   = false;

    std::function<void(int)> m_onPageChange;

    struct Impl;
    std::shared_ptr<Impl> m_impl;

    void ensureInitialized();
    void navigateDocument();

    static constexpr int kBarH = 30;

    SkRect pageArea() const;
    SkRect toolbar()  const;
    SkRect prevBtn()  const;
    SkRect nextBtn()  const;
};
