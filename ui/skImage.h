#pragma once
#include "skWidget.h"
#include <string>
#include <include/core/SkImage.h>
#include <memory>

enum class skScaleMode { Stretch, Fit, Fill, Center };

class skImage : public skWidget {
public:
    skImage(int x, int y, int w, int h);

    static std::shared_ptr<skImage> make(int x, int y, int w, int h);

    // Load PNG / JPEG from the file system. Returns false if file not found or decoding fails.
    bool loadFromFile(const std::string& path);

    void setScaleMode(skScaleMode mode) { m_mode = mode; }
    void setPlaceholder(const std::string& text) { m_placeholder = text; }

    void Paint(SkCanvas* canvas) override;

    std::shared_ptr<skImage> scaleMode(skScaleMode mode);
    std::shared_ptr<skImage> placeholder(const std::string& t);
    std::shared_ptr<skImage> pos(int px, int py);
    std::shared_ptr<skImage> size(int pw, int ph);

private:
    std::shared_ptr<skImage> shared_this() {
        return std::static_pointer_cast<skImage>(shared_from_this());
    }

    sk_sp<SkImage> m_image;
    skScaleMode    m_mode        = skScaleMode::Fit;
    std::string    m_placeholder = "No image";
};
