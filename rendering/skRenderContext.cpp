#include "skRenderContext.h"
#include <include/core/SkImageInfo.h>

void skRenderContext::resize(int width, int height) {
    m_width  = width;
    m_height = height;
    m_surface = SkSurfaces::Raster(SkImageInfo::MakeN32Premul(width, height));
}

void skRenderContext::clear(SkColor color) {
    if (m_surface) m_surface->getCanvas()->clear(color);
}

SkCanvas* skRenderContext::getCanvas() {
    return m_surface ? m_surface->getCanvas() : nullptr;
}

bool skRenderContext::readPixels(SkPixmap& pixmap) {
    return m_surface && m_surface->peekPixels(&pixmap);
}
