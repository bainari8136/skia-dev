#include "skImage.h"
#include "skTypeface.h"
#include "skTheme.h"
#include <include/core/SkData.h>
#include <include/core/SkPaint.h>
#include <include/core/SkFont.h>
#include <include/core/SkSamplingOptions.h>
#include <algorithm>
#include <cstdio>
#include <vector>

skImage::skImage(int ix, int iy, int iw, int ih) : skWidget(ix, iy, iw, ih) {}

bool skImage::loadFromFile(const std::string& path) {
    FILE* f = fopen(path.c_str(), "rb");
    if (!f) return false;
    fseek(f, 0, SEEK_END);
    long sz = ftell(f);
    fseek(f, 0, SEEK_SET);
    if (sz <= 0) { fclose(f); return false; }
    std::vector<uint8_t> buf((size_t)sz);
    fread(buf.data(), 1, (size_t)sz, f);
    fclose(f);

    auto data = SkData::MakeWithCopy(buf.data(), buf.size());
    m_image = SkImages::DeferredFromEncodedData(data);
    return m_image != nullptr;
}

void skImage::Paint(SkCanvas* canvas) {
    const auto& th = skGetTheme();
    SkRect dst = SkRect::MakeXYWH((float)x, (float)y, (float)w, (float)h);

    // Background
    SkPaint bg;
    bg.setAntiAlias(true);
    bg.setColor(th.inputBg);
    canvas->drawRect(dst, bg);

    if (!m_image) {
        // Placeholder: crossed lines + text
        SkPaint xp;
        xp.setAntiAlias(true);
        xp.setColor(th.inputBorder);
        xp.setStrokeWidth(1.f);
        canvas->drawLine(dst.left(), dst.top(), dst.right(), dst.bottom(), xp);
        canvas->drawLine(dst.right(), dst.top(), dst.left(), dst.bottom(), xp);

        if (!m_placeholder.empty()) {
            static sk_sp<SkTypeface> s_tf = skGetSystemTypeface();
            SkFont font(s_tf, 11.f);
            font.setEdging(SkFont::Edging::kAntiAlias);
            SkRect tb;
            font.measureText(m_placeholder.c_str(), m_placeholder.size(),
                             SkTextEncoding::kUTF8, &tb);
            SkPaint tp;
            tp.setAntiAlias(true);
            tp.setColor(th.textPlaceholder);
            canvas->drawString(m_placeholder.c_str(),
                (float)x + ((float)w - tb.width()) / 2.f - tb.left(),
                (float)y + ((float)h - tb.height()) / 2.f - tb.top(),
                font, tp);
        }
    } else {
        SkSamplingOptions sampling(SkFilterMode::kLinear);
        float iw = (float)m_image->width();
        float ih = (float)m_image->height();

        switch (m_mode) {
            case skScaleMode::Stretch:
                canvas->drawImageRect(m_image.get(), dst, sampling);
                break;

            case skScaleMode::Fit: {
                float scale = std::min((float)w / iw, (float)h / ih);
                float fw = iw * scale, fh = ih * scale;
                SkRect fitDst = SkRect::MakeXYWH(
                    (float)x + ((float)w - fw) / 2.f,
                    (float)y + ((float)h - fh) / 2.f,
                    fw, fh);
                canvas->drawImageRect(m_image.get(), fitDst, sampling);
                break;
            }

            case skScaleMode::Fill: {
                float scale = std::max((float)w / iw, (float)h / ih);
                SkRect src = SkRect::MakeXYWH(
                    (iw * scale - (float)w) / (2.f * scale),
                    (ih * scale - (float)h) / (2.f * scale),
                    (float)w / scale, (float)h / scale);
                canvas->save();
                canvas->clipRect(dst);
                canvas->drawImageRect(m_image.get(), src, dst, sampling, nullptr,
                                      SkCanvas::kStrict_SrcRectConstraint);
                canvas->restore();
                break;
            }

            case skScaleMode::Center: {
                SkRect centDst = SkRect::MakeXYWH(
                    (float)x + ((float)w - iw) / 2.f,
                    (float)y + ((float)h - ih) / 2.f,
                    iw, ih);
                canvas->save();
                canvas->clipRect(dst);
                canvas->drawImageRect(m_image.get(), centDst, sampling);
                canvas->restore();
                break;
            }
        }
    }

    // Border
    SkPaint brd;
    brd.setAntiAlias(true);
    brd.setStyle(SkPaint::kStroke_Style);
    brd.setStrokeWidth(1.f);
    brd.setColor(th.inputBorder);
    canvas->drawRect(dst, brd);
}
