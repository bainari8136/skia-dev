#include <Windows.h>
#include "skNumberInput.h"
#include "skTypeface.h"
#include "skTheme.h"
#include <include/core/SkPaint.h>
#include <include/core/SkFont.h>
#include <include/core/SkRRect.h>
#include <algorithm>

skNumberInput::skNumberInput(int nx, int ny, int nw, int nh, int mn, int mx, int val)
    : skWidget(nx, ny, nw, nh), m_min(mn), m_max(mx), m_value(val) {}

std::shared_ptr<skNumberInput> skNumberInput::make(int nx, int ny, int nw, int nh,
                                                    int minVal, int maxVal, int val) {
    return std::make_shared<skNumberInput>(nx, ny, nw, nh, minVal, maxVal, val);
}
std::shared_ptr<skNumberInput> skNumberInput::withValue(int v)                       { setValue(v); return shared_this(); }
std::shared_ptr<skNumberInput> skNumberInput::range(int mn, int mx)                  { m_min = mn; m_max = mx; setValue(m_value); return shared_this(); }
std::shared_ptr<skNumberInput> skNumberInput::step(int s)                            { m_step = s; return shared_this(); }
std::shared_ptr<skNumberInput> skNumberInput::onChange(std::function<void(int)> cb)  { m_onChange = std::move(cb); return shared_this(); }
std::shared_ptr<skNumberInput> skNumberInput::pos(int px, int py)                    { x = px; y = py; return shared_this(); }
std::shared_ptr<skNumberInput> skNumberInput::size(int pw, int ph)                   { w = pw; h = ph; return shared_this(); }

SkRect skNumberInput::minusRect() const {
    return SkRect::MakeXYWH((float)x, (float)y, (float)kBtnW, (float)h);
}
SkRect skNumberInput::plusRect() const {
    return SkRect::MakeXYWH((float)(x + w - kBtnW), (float)y, (float)kBtnW, (float)h);
}

void skNumberInput::setValue(int v) {
    m_value = std::max(m_min, std::min(m_max, v));
    if (m_onChange) m_onChange(m_value);
}

void skNumberInput::increment(int delta) { setValue(m_value + delta); }

void skNumberInput::Paint(SkCanvas* canvas) {
    const auto& th = skGetTheme();

    // Full background
    SkRRect fullRR;
    fullRR.setRectXY(SkRect::MakeXYWH((float)x, (float)y, (float)w, (float)h), 6.f, 6.f);
    SkPaint bgP;
    bgP.setAntiAlias(true);
    bgP.setColor(th.inputBg);
    canvas->drawRRect(fullRR, bgP);

    // Left [−] button with rounded left corners
    {
        SkVector lc[4] = {{6,6},{0,0},{0,0},{6,6}};
        SkRRect lrr;
        lrr.setRectRadii(SkRect::MakeXYWH((float)x, (float)y, (float)kBtnW, (float)h), lc);
        SkPaint lp;
        lp.setAntiAlias(true);
        lp.setColor(m_minusPress ? th.accentPress : (m_minusHov ? th.accentHover : th.accent));
        canvas->drawRRect(lrr, lp);
    }

    // Right [+] button with rounded right corners
    {
        SkVector rc[4] = {{0,0},{6,6},{6,6},{0,0}};
        SkRRect rrr;
        rrr.setRectRadii(SkRect::MakeXYWH((float)(x+w-kBtnW), (float)y, (float)kBtnW, (float)h), rc);
        SkPaint rp;
        rp.setAntiAlias(true);
        rp.setColor(m_plusPress ? th.accentPress : (m_plusHov ? th.accentHover : th.accent));
        canvas->drawRRect(rrr, rp);
    }

    static sk_sp<SkTypeface> s_tf = skGetSystemTypeface();
    SkFont symFont(s_tf, 17.f);
    symFont.setEdging(SkFont::Edging::kAntiAlias);
    SkPaint symP;
    symP.setAntiAlias(true);
    symP.setColor(th.textOnAccent);

    // "−" symbol in left button
    {
        const char* sym = "-";
        SkRect sb;
        symFont.measureText(sym, 1, SkTextEncoding::kUTF8, &sb);
        float sx = (float)x + ((float)kBtnW - sb.width()) / 2.f - sb.left();
        float sy = (float)y + ((float)h - sb.height()) / 2.f - sb.top();
        canvas->drawString(sym, sx, sy, symFont, symP);
    }
    // "+" symbol in right button
    {
        const char* sym = "+";
        SkRect sb;
        symFont.measureText(sym, 1, SkTextEncoding::kUTF8, &sb);
        float sx = (float)(x+w-kBtnW) + ((float)kBtnW - sb.width()) / 2.f - sb.left();
        float sy = (float)y + ((float)h - sb.height()) / 2.f - sb.top();
        canvas->drawString(sym, sx, sy, symFont, symP);
    }

    // Value text in center
    {
        std::string val = std::to_string(m_value);
        SkFont vFont(s_tf, 14.f);
        vFont.setEdging(SkFont::Edging::kAntiAlias);
        SkRect vb;
        vFont.measureText(val.c_str(), val.size(), SkTextEncoding::kUTF8, &vb);
        float vx = (float)(x + kBtnW) + ((float)(w - 2*kBtnW) - vb.width()) / 2.f - vb.left();
        float vy = (float)y + ((float)h - vb.height()) / 2.f - vb.top();
        SkPaint vp;
        vp.setAntiAlias(true);
        vp.setColor(th.textPrimary);
        canvas->drawString(val.c_str(), vx, vy, vFont, vp);
    }

    // Divider lines between buttons and center
    SkPaint div;
    div.setColor(SkColorSetARGB(50, 0, 0, 0));
    canvas->drawLine((float)(x+kBtnW),   (float)y+2, (float)(x+kBtnW),   (float)(y+h-2), div);
    canvas->drawLine((float)(x+w-kBtnW), (float)y+2, (float)(x+w-kBtnW), (float)(y+h-2), div);

    // Border
    SkPaint brd;
    brd.setAntiAlias(true);
    brd.setStyle(SkPaint::kStroke_Style);
    brd.setStrokeWidth(m_focused ? 2.f : 1.f);
    brd.setColor(m_focused ? th.accent : th.inputBorder);
    canvas->drawRRect(fullRR, brd);
}

void skNumberInput::OnEvent(const skEvent& event) {
    SkRect lr = minusRect(), rr = plusRect();
    switch (event.type) {
        case skEventType::MouseMove:
            m_minusHov = lr.contains((float)event.x, (float)event.y);
            m_plusHov  = rr.contains((float)event.x, (float)event.y);
            break;
        case skEventType::MouseDown:
            m_minusPress = lr.contains((float)event.x, (float)event.y);
            m_plusPress  = rr.contains((float)event.x, (float)event.y);
            break;
        case skEventType::MouseUp:
            if (m_minusPress && lr.contains((float)event.x, (float)event.y)) increment(-m_step);
            if (m_plusPress  && rr.contains((float)event.x, (float)event.y)) increment(+m_step);
            m_minusPress = m_plusPress = false;
            break;
        case skEventType::KeyDown:
            switch (event.button) {
                case VK_UP:   increment(+m_step); break;
                case VK_DOWN: increment(-m_step); break;
            }
            break;
        default: break;
    }
}
