#include <Windows.h>
#include "skDatePicker.h"
#include "skTheme.h"
#include "skTypeface.h"
#include <include/core/SkPaint.h>
#include <include/core/SkFont.h>
#include <include/core/SkRRect.h>
#include <cstdio>
#include <cstring>
#include <ctime>
#include <algorithm>

static sk_sp<SkTypeface>& dpTf() {
    static sk_sp<SkTypeface> tf = skGetSystemTypeface();
    return tf;
}

int skDatePicker::daysInMonth(int y, int m) {
    if (m == 2) return (y%4==0 && (y%100!=0 || y%400==0)) ? 29 : 28;
    const int d[] = {31,28,31,30,31,30,31,31,30,31,30,31};
    return d[m-1];
}

int skDatePicker::firstWeekday(int y, int m) {
    struct tm t = {};
    t.tm_year = y - 1900; t.tm_mon = m - 1; t.tm_mday = 1;
    mktime(&t);
    return t.tm_wday; // 0=Sun
}

const char* skDatePicker::monthName(int m) {
    static const char* names[] = {
        "January","February","March","April","May","June",
        "July","August","September","October","November","December"
    };
    if (m >= 1 && m <= 12) return names[m-1];
    return "";
}

void skDatePicker::prevMonth() {
    if (--m_view.month < 1) { m_view.month = 12; --m_view.year; }
}
void skDatePicker::nextMonth() {
    if (++m_view.month > 12) { m_view.month = 1; ++m_view.year; }
}

skDatePicker::skDatePicker(int winW, int winH)
    : skWidget(0, 0, winW, winH) {
    setVisible(false);
    time_t now = time(nullptr);
    struct tm* lt = localtime(&now);
    m_today = { 1900 + lt->tm_year, 1 + lt->tm_mon, lt->tm_mday };
}

void skDatePicker::show(skDate d) {
    if (d.year == 0) d = m_today;
    m_sel = m_view = d;
    m_okHov = m_cancelHov = m_prevHov = m_nextHov = false;
    setVisible(true);
}

SkRect skDatePicker::prevRect() const {
    return SkRect::MakeXYWH(dlgX() + 8.f, dlgY() + (kHeaderH-28.f)/2.f, 28.f, 28.f);
}
SkRect skDatePicker::nextRect() const {
    return SkRect::MakeXYWH(dlgX() + kDlgW - 36.f, dlgY() + (kHeaderH-28.f)/2.f, 28.f, 28.f);
}
SkRect skDatePicker::okRect() const {
    return SkRect::MakeXYWH(dlgX() + kDlgW - kBtnW - 10.f,
                             dlgY() + kDlgH - kBtnH - 10.f, kBtnW, kBtnH);
}
SkRect skDatePicker::cancelRect() const {
    SkRect ok = okRect();
    return SkRect::MakeXYWH(ok.left() - kBtnW - 8.f, ok.top(), kBtnW, kBtnH);
}

int skDatePicker::dayAt(int px, int py) const {
    float gx = gridX(), gy = gridY();
    float lx = (float)px - gx, ly = (float)py - gy;
    if (lx < 0.f || ly < 0.f || lx >= 7.f*kCellW || ly >= 6.f*kCellH) return 0;
    int col = (int)(lx / kCellW);
    int row = (int)(ly / kCellH);
    int fd  = firstWeekday(m_view.year, m_view.month);
    int day = row * 7 + col - fd + 1;
    int dim = daysInMonth(m_view.year, m_view.month);
    if (day < 1 || day > dim) return 0;
    return day;
}

void skDatePicker::Paint(SkCanvas* canvas) {
    const auto& th = skGetTheme();
    float dx = dlgX(), dy = dlgY();

    // Backdrop
    SkPaint bk; bk.setColor(SkColorSetARGB(130, 0, 0, 0));
    canvas->drawRect(SkRect::MakeXYWH((float)x,(float)y,(float)w,(float)h), bk);

    // Shadow
    SkRRect sh; sh.setRectXY(SkRect::MakeXYWH(dx+2.f,dy+5.f,(float)kDlgW,(float)kDlgH), kR, kR);
    SkPaint shP; shP.setAntiAlias(true); shP.setColor(SkColorSetARGB(28,0,0,0));
    canvas->drawRRect(sh, shP);

    // Dialog background
    SkRRect rr; rr.setRectXY(SkRect::MakeXYWH(dx,dy,(float)kDlgW,(float)kDlgH), kR, kR);
    SkPaint bgP; bgP.setAntiAlias(true); bgP.setColor(th.panelBg);
    canvas->drawRRect(rr, bgP);

    // Accent top bar
    canvas->save();
    canvas->clipRRect(rr, SkClipOp::kIntersect, true);
    SkPaint bar; bar.setColor(th.accent);
    canvas->drawRect(SkRect::MakeXYWH(dx, dy, (float)kDlgW, 5.f), bar);
    canvas->restore();

    // Border
    SkPaint brd; brd.setAntiAlias(true);
    brd.setStyle(SkPaint::kStroke_Style); brd.setStrokeWidth(0.75f);
    brd.setColor(th.panelBorder);
    canvas->drawRRect(rr, brd);

    SkFont font(dpTf(), 13.f);  font.setEdging(SkFont::Edging::kAntiAlias);
    SkFont small(dpTf(), 11.f); small.setEdging(SkFont::Edging::kAntiAlias);
    SkFont tiny(dpTf(),  10.f); tiny.setEdging(SkFont::Edging::kAntiAlias);

    // Header: month/year
    {
        char hdr[32]; snprintf(hdr, sizeof(hdr), "%s %d", monthName(m_view.month), m_view.year);
        SkRect hb; font.measureText(hdr, strlen(hdr), SkTextEncoding::kUTF8, &hb);
        SkPaint hp; hp.setAntiAlias(true); hp.setColor(th.textPrimary);
        canvas->drawString(hdr,
            dx + (float)kDlgW/2.f - hb.width()/2.f - hb.left(),
            dy + kHeaderH/2.f - hb.height()/2.f - hb.top(),
            font, hp);
    }

    // Prev/Next buttons
    auto drawArrow = [&](SkRect r, const char* ch, bool hov) {
        SkRRect arr; arr.setRectXY(r, 4.f, 4.f);
        SkPaint ap; ap.setAntiAlias(true);
        ap.setColor(hov ? th.accentHover : SkColorSetARGB(0,0,0,0));
        canvas->drawRRect(arr, ap);
        SkRect tb; small.measureText(ch, strlen(ch), SkTextEncoding::kUTF8, &tb);
        SkPaint tp; tp.setAntiAlias(true); tp.setColor(hov ? th.accent : th.textSecondary);
        canvas->drawString(ch,
            r.centerX() - tb.width()/2.f - tb.left(),
            r.centerY() - tb.height()/2.f - tb.top(),
            small, tp);
    };
    drawArrow(prevRect(), "<", m_prevHov);
    drawArrow(nextRect(), ">", m_nextHov);

    // Weekday headers
    const char* days[] = {"Su","Mo","Tu","We","Th","Fr","Sa"};
    float gx = gridX(), gy = gridY();
    float wky = dy + kHeaderH + kWeekH/2.f;
    for (int i = 0; i < 7; ++i) {
        SkRect db; tiny.measureText(days[i], 2, SkTextEncoding::kUTF8, &db);
        SkPaint dp2; dp2.setAntiAlias(true); dp2.setColor(th.textSecondary);
        canvas->drawString(days[i],
            gx + (float)i*kCellW + kCellW/2.f - db.width()/2.f - db.left(),
            wky - db.height()/2.f - db.top(), tiny, dp2);
    }

    // Day separator line
    SkPaint sep; sep.setAntiAlias(true);
    sep.setStyle(SkPaint::kStroke_Style); sep.setStrokeWidth(1.f);
    sep.setColor(th.panelBorder);
    canvas->drawLine(dx+8.f, dy+kHeaderH+kWeekH, dx+(float)kDlgW-8.f, dy+kHeaderH+kWeekH, sep);

    // Day cells
    int fd  = firstWeekday(m_view.year, m_view.month);
    int dim = daysInMonth(m_view.year, m_view.month);
    for (int day = 1; day <= dim; ++day) {
        int idx = day - 1 + fd;
        int row = idx / 7, col = idx % 7;
        float cx = gx + (float)col*kCellW + kCellW/2.f;
        float cy = gy + (float)row*kCellH + kCellH/2.f;
        float r2 = kCellH/2.f - 3.f;

        bool isSel   = (day==m_sel.day && m_view.month==m_sel.month && m_view.year==m_sel.year);
        bool isToday = (day==m_today.day && m_view.month==m_today.month && m_view.year==m_today.year);

        if (isSel) {
            SkPaint cp; cp.setAntiAlias(true); cp.setColor(th.accent);
            canvas->drawCircle(cx, cy, r2, cp);
        } else if (isToday) {
            SkPaint cp; cp.setAntiAlias(true);
            cp.setStyle(SkPaint::kStroke_Style); cp.setStrokeWidth(1.5f);
            cp.setColor(th.accent);
            canvas->drawCircle(cx, cy, r2, cp);
        }

        char d2[4]; snprintf(d2, sizeof(d2), "%d", day);
        SkRect db; tiny.measureText(d2, strlen(d2), SkTextEncoding::kUTF8, &db);
        SkPaint dp3; dp3.setAntiAlias(true);
        dp3.setColor(isSel ? th.textOnAccent : (col==0||col==6 ? th.textSecondary : th.textPrimary));
        canvas->drawString(d2,
            cx - db.width()/2.f - db.left(),
            cy - db.height()/2.f - db.top(), tiny, dp3);
    }

    // OK / Cancel buttons
    auto drawBtn = [&](SkRect r, const char* lbl, bool filled, bool hov) {
        SkRRect brr; brr.setRectXY(r, 5.f, 5.f);
        SkPaint bp; bp.setAntiAlias(true);
        bp.setColor(filled ? (hov ? th.accentHover : th.accent) : th.panelBg);
        canvas->drawRRect(brr, bp);
        SkPaint brd2; brd2.setAntiAlias(true);
        brd2.setStyle(SkPaint::kStroke_Style); brd2.setStrokeWidth(1.f);
        brd2.setColor(filled ? SkColorSetARGB(60,0,0,0) : (hov ? th.accent : th.inputBorder));
        canvas->drawRRect(brr, brd2);
        SkRect lb; small.measureText(lbl, strlen(lbl), SkTextEncoding::kUTF8, &lb);
        SkPaint lp; lp.setAntiAlias(true);
        lp.setColor(filled ? th.textOnAccent : (hov ? th.accent : th.textPrimary));
        canvas->drawString(lbl,
            r.centerX() - lb.width()/2.f - lb.left(),
            r.centerY() - lb.height()/2.f - lb.top(),
            small, lp);
    };
    drawBtn(okRect(),     "OK",     true,  m_okHov);
    drawBtn(cancelRect(), "Cancel", false, m_cancelHov);
}

bool skDatePicker::handleEvent(const skEvent& ev) {
    if (!visible()) return false;

    if (ev.type == skEventType::KeyDown && ev.button == VK_ESCAPE) {
        setVisible(false); if (m_onCancel) m_onCancel(); return true;
    }

    SkRect okR = okRect(), canR = cancelRect(), prR = prevRect(), nxR = nextRect();

    if (ev.type == skEventType::MouseMove) {
        m_okHov     = okR.contains((float)ev.x,(float)ev.y);
        m_cancelHov = canR.contains((float)ev.x,(float)ev.y);
        m_prevHov   = prR.contains((float)ev.x,(float)ev.y);
        m_nextHov   = nxR.contains((float)ev.x,(float)ev.y);
        return true;
    }

    if (ev.type == skEventType::MouseDown) {
        if (prR.contains((float)ev.x,(float)ev.y)) { prevMonth(); return true; }
        if (nxR.contains((float)ev.x,(float)ev.y)) { nextMonth(); return true; }
        int d = dayAt(ev.x, ev.y);
        if (d > 0) { m_sel = {m_view.year, m_view.month, d}; return true; }
        if (okR.contains((float)ev.x,(float)ev.y)) {
            setVisible(false); if (m_onConfirm) m_onConfirm(m_sel); return true;
        }
        if (canR.contains((float)ev.x,(float)ev.y)) {
            setVisible(false); if (m_onCancel) m_onCancel(); return true;
        }
        // Click outside dialog
        float ddx = dlgX(), ddy = dlgY();
        bool inDlg = (ev.x>=(int)ddx && ev.x<(int)(ddx+kDlgW) &&
                      ev.y>=(int)ddy && ev.y<(int)(ddy+kDlgH));
        if (!inDlg) { setVisible(false); if (m_onCancel) m_onCancel(); }
        return true;
    }
    return true;
}
