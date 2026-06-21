#pragma once
#include <include/core/SkSurface.h>
#include <include/core/SkCanvas.h>
#include <include/core/SkPixmap.h>
#include <include/core/SkColor.h>

class skRenderContext {
public:
    void resize(int width, int height);
    void clear(SkColor color = SK_ColorWHITE);
    SkCanvas* getCanvas();
    bool readPixels(SkPixmap& pixmap);
    int width()  const { return m_width; }
    int height() const { return m_height; }
    bool valid() const { return m_surface != nullptr; }

private:
    sk_sp<SkSurface> m_surface;
    int m_width  = 0;
    int m_height = 0;
};
