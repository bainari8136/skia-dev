#pragma once
#include "skWidget.h"
#include <string>
#include <include/core/SkImage.h>

enum class skScaleMode { Stretch, Fit, Fill, Center };

class skImage : public skWidget {
public:
    skImage(int x, int y, int w, int h);

    // Load PNG / JPEG from the file system. Returns false if file not found or decoding fails.
    bool loadFromFile(const std::string& path);

    void setScaleMode(skScaleMode mode) { m_mode = mode; }
    void setPlaceholder(const std::string& text) { m_placeholder = text; }

    void Paint(SkCanvas* canvas) override;

private:
    sk_sp<SkImage> m_image;
    skScaleMode    m_mode        = skScaleMode::Fit;
    std::string    m_placeholder = "No image";
};
