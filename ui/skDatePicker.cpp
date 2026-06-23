#include <Windows.h>
#include "skDatePicker.h"
#include "skTheme.h"
#include "skTypeface.h"
#include <include/core/SkPaint.h>
#include <include/core/SkFont.h>
#include <include/core/SkPath.h>
#include <include/core/SkRRect.h>
#include <cstdio>
#include <cstring>
#include <ctime>

static sk_sp<SkTypeface>& dpTf() {
    static sk_sp<SkTypeface> tf = skGetSystemTypeface();
    return tf;
}

static SkColor withAlpha(SkColor c, uint8_t a) {
    return SkColorSetARGB(a, SkColorGetR(c), SkColorGetG(c), SkColorGetB(c));
}

int skDatePicker::daysInMonth(int y, int m) {
    if (m == 2) return (y % 4 == 0 && (y % 100 != 0 || y % 400 == 0)) ? 29 : 28;
    const int d[] = {31,28,31,30,31,30,31,31,30,31,30,31};
    return d[m - 1];
}

int skDatePicker::firstWeekday(int y, int m) {
    struct tm t = {};
    t.tm_year = y - 1900; t.tm_mon = m - 1; t.tm_mday = 1;
    mktime(&t);
    return t.tm_wday;
}

const char* skDatePicker::monthName(int m) {
    static const char* names[] = {
        "January","February","March","April","May","June",
        "July","August","September","October","November","December"
    };
    return (m >= 1 && m <= 12) ? names[m - 1] : "";
}

static const char* monthShort(int m) {
    static const char* n[] = {
        "Jan","Feb","Mar","Apr","May","Jun",
        "Jul","Aug","Sep","Oct","Nov","Dec"
    };
    return (m >= 1 && m <= 12) ? n[m - 1] : "";
}

static const char* wdayShort(int wd) {
    static const char* n[] = {"Sun","Mon","Tue","Wed","Thu","Fri","Sat"};
    return n[wd % 7];
}

static int weekdayOf(int y, int m, int d) {
    struct tm t = {};
    t.tm_year = y - 1900; t.tm_mon = m - 1; t.tm_mday = d;
    mktime(&t);
    return t.tm_wday;
}

void skDatePicker::prevPage() {
    if (m_mode == skDPMode::YearPicker)
        m_yearPage -= kYearsPerPg;
    else {
        if (--m_view.month < 1) { m_view.month = 12; --m_view.year; }
    }
}
void skDatePicker::nextPage() {
    if (m_mode == skDPMode::YearPicker)
        m_yearPage += kYearsPerPg;
    else {
        if (++m_view.month > 12) { m_view.month = 1; ++m_view.year; }
    }
}

skDatePicker::skDatePicker(int winW, int winH) : skWidget(0, 0, winW, winH) {
    setVisible(false);
    time_t now = time(nullptr);
    struct tm* lt = localtime(&now);
    m_today = { 1900 + lt->tm_year, 1 + lt->tm_mon, lt->tm_mday };
}

void skDatePicker::show(skDate d) {
    if (d.year == 0) d = m_today;
    m_sel = m_view = d;
    m_mode     = skDPMode::Calendar;
    m_yearPage = (m_view.year / kYearsPerPg) * kYearsPerPg;
    m_okHov = m_cancelHov = m_prevHov = m_nextHov = m_labelHov = false;
    m_hovDay = m_hovYear = 0;
    setVisible(true);
}

SkRect skDatePicker::prevRect() const {
    float cy = dlgY() + kHdrH + kNavH * 0.5f;
    return SkRect::MakeXYWH(dlgX() + 8.f, cy - 20.f, 40.f, 40.f);
}
SkRect skDatePicker::nextRect() const {
    float cy = dlgY() + kHdrH + kNavH * 0.5f;
    return SkRect::MakeXYWH(dlgX() + kDlgW - 48.f, cy - 20.f, 40.f, 40.f);
}
SkRect skDatePicker::okRect() const {
    float actY = dlgY() + kDlgH - kActH;
    return SkRect::MakeXYWH(dlgX() + kDlgW - 84.f, actY + (kActH - 36.f) * 0.5f, 72.f, 36.f);
}
SkRect skDatePicker::cancelRect() const {
    SkRect ok = okRect();
    return SkRect::MakeXYWH(ok.left() - 88.f, ok.top(), 80.f, ok.height());
}
SkRect skDatePicker::monthYearRect() const {
    float cy = dlgY() + kHdrH + kNavH * 0.5f;
    float cx = dlgX() + kDlgW * 0.5f;
    return SkRect::MakeXYWH(cx - 80.f, cy - 18.f, 160.f, 36.f);
}

int skDatePicker::dayAt(int px, int py) const {
    float gx = gridX(), gy = gridY();
    float lx = (float)px - gx, ly = (float)py - gy;
    if (lx < 0.f || ly < 0.f || lx >= 7.f * kCellW || ly >= 6.f * kCellH) return 0;
    int col = (int)(lx / kCellW);
    int row = (int)(ly / kCellH);
    int fd  = firstWeekday(m_view.year, m_view.month);
    int day = row * 7 + col - fd + 1;
    int dim = daysInMonth(m_view.year, m_view.month);
    return (day >= 1 && day <= dim) ? day : 0;
}

int skDatePicker::yearAt(int px, int py) const {
    float gx = yearGridX(), gy = yearGridY();
    float lx = (float)px - gx, ly = (float)py - gy;
    if (lx < 0.f || ly < 0.f ||
        lx >= kYearCols * kYearCellW || ly >= 5.f * kYearCellH) return 0;
    int col = (int)(lx / kYearCellW);
    int row = (int)(ly / kYearCellH);
    return m_yearPage + row * kYearCols + col;
}

void skDatePicker::drawChevron(SkCanvas* canvas, float cx, float cy, bool left, SkColor color) const {
    const float s = 5.5f;
    SkPath path;
    if (left) {
        path.moveTo(cx + s, cy - s * 1.2f);
        path.lineTo(cx - s, cy);
        path.lineTo(cx + s, cy + s * 1.2f);
    } else {
        path.moveTo(cx - s, cy - s * 1.2f);
        path.lineTo(cx + s, cy);
        path.lineTo(cx - s, cy + s * 1.2f);
    }
    SkPaint p; p.setAntiAlias(true);
    p.setStyle(SkPaint::kStroke_Style); p.setStrokeWidth(2.f);
    p.setStrokeCap(SkPaint::kRound_Cap); p.setStrokeJoin(SkPaint::kRound_Join);
    p.setColor(color);
    canvas->drawPath(path, p);
}

void skDatePicker::drawDropArrow(SkCanvas* canvas, float cx, float cy, bool up, SkColor color) const {
    const float s = 4.f;
    SkPath p;
    if (up) {
        p.moveTo(cx - s, cy + s * 0.6f);
        p.lineTo(cx,     cy - s * 0.6f);
        p.lineTo(cx + s, cy + s * 0.6f);
    } else {
        p.moveTo(cx - s, cy - s * 0.6f);
        p.lineTo(cx,     cy + s * 0.6f);
        p.lineTo(cx + s, cy - s * 0.6f);
    }
    SkPaint paint; paint.setAntiAlias(true);
    paint.setStyle(SkPaint::kStroke_Style); paint.setStrokeWidth(2.f);
    paint.setStrokeCap(SkPaint::kRound_Cap); paint.setStrokeJoin(SkPaint::kRound_Join);
    paint.setColor(color);
    canvas->drawPath(p, paint);
}

void skDatePicker::Paint(SkCanvas* canvas) {
    const auto& th = skGetTheme();
    float dx = dlgX(), dy = dlgY();

    // Scrim
    SkPaint scrimP; scrimP.setColor(SkColorSetARGB(130, 0, 0, 0));
    canvas->drawRect(SkRect::MakeXYWH((float)x, (float)y, (float)w, (float)h), scrimP);

    // Shadow
    {
        SkRRect sh; sh.setRectXY(SkRect::MakeXYWH(dx + 3.f, dy + 8.f, (float)kDlgW, (float)kDlgH), kR, kR);
        SkPaint shP; shP.setAntiAlias(true); shP.setColor(SkColorSetARGB(30, 0, 0, 0));
        canvas->drawRRect(sh, shP);
    }
    {
        SkRRect sh; sh.setRectXY(SkRect::MakeXYWH(dx + 1.f, dy + 2.f, (float)kDlgW, (float)kDlgH), kR, kR);
        SkPaint shP; shP.setAntiAlias(true); shP.setColor(SkColorSetARGB(15, 0, 0, 0));
        canvas->drawRRect(sh, shP);
    }

    // Dialog surface
    SkRect dlgRect = SkRect::MakeXYWH(dx, dy, (float)kDlgW, (float)kDlgH);
    SkRRect rr; rr.setRectXY(dlgRect, kR, kR);
    SkPaint bgP; bgP.setAntiAlias(true); bgP.setColor(th.panelBg);
    canvas->drawRRect(rr, bgP);

    canvas->save();
    canvas->clipRRect(rr, SkClipOp::kIntersect, true);

    SkFont labelSm (dpTf(), 12.f); labelSm.setEdging(SkFont::Edging::kAntiAlias);
    SkFont headline(dpTf(), 32.f); headline.setEdging(SkFont::Edging::kAntiAlias);
    SkFont titleSm (dpTf(), 14.f); titleSm.setEdging(SkFont::Edging::kAntiAlias);
    SkFont bodyMed (dpTf(), 14.f); bodyMed.setEdging(SkFont::Edging::kAntiAlias);
    SkFont bodySm  (dpTf(), 12.f); bodySm.setEdging(SkFont::Edging::kAntiAlias);
    SkFont lblLarge(dpTf(), 14.f); lblLarge.setEdging(SkFont::Edging::kAntiAlias);

    // ── HEADER ──────────────────────────────────────────────────────────────
    {
        const char* title = (m_mode == skDPMode::YearPicker) ? "Select year" : "Select date";
        SkRect tb; labelSm.measureText(title, strlen(title), SkTextEncoding::kUTF8, &tb);
        SkPaint tp; tp.setAntiAlias(true); tp.setColor(withAlpha(th.textPrimary, 153));
        canvas->drawString(title, dx + 24.f, dy + 24.f - tb.top(), labelSm, tp);
    }
    {
        int wd = weekdayOf(m_sel.year, m_sel.month, m_sel.day);
        char buf[32];
        snprintf(buf, sizeof(buf), "%s, %s %d", wdayShort(wd), monthShort(m_sel.month), m_sel.day);
        SkRect hb; headline.measureText(buf, strlen(buf), SkTextEncoding::kUTF8, &hb);
        SkPaint hp; hp.setAntiAlias(true); hp.setColor(th.textPrimary);
        canvas->drawString(buf, dx + 24.f, dy + 52.f - hb.top(), headline, hp);
    }
    {
        SkPaint div; div.setAntiAlias(true);
        div.setStyle(SkPaint::kStroke_Style); div.setStrokeWidth(1.f);
        div.setColor(withAlpha(th.textPrimary, 30));
        canvas->drawLine(dx, dy + kHdrH, dx + kDlgW, dy + kHdrH, div);
    }

    // ── NAV ROW ─────────────────────────────────────────────────────────────
    float navCY = dy + kHdrH + kNavH * 0.5f;

    auto drawNavBtn = [&](SkRect r, bool left, bool hov) {
        if (hov) {
            SkPaint hp; hp.setAntiAlias(true); hp.setColor(withAlpha(th.accent, 20));
            canvas->drawCircle(r.centerX(), r.centerY(), 20.f, hp);
        }
        SkColor chCol = hov ? th.accent : withAlpha(th.textPrimary, 160);
        drawChevron(canvas, r.centerX(), r.centerY(), left, chCol);
    };
    drawNavBtn(prevRect(), true,  m_prevHov);
    drawNavBtn(nextRect(), false, m_nextHov);

    // Clickable month/year label (shows current month+year or year range)
    {
        char mv[40];
        if (m_mode == skDPMode::YearPicker)
            snprintf(mv, sizeof(mv), "%d \xe2\x80\x93 %d",   // en-dash
                m_yearPage, m_yearPage + kYearsPerPg - 1);
        else
            snprintf(mv, sizeof(mv), "%s %d", monthName(m_view.month), m_view.year);

        SkRect mb; titleSm.measureText(mv, strlen(mv), SkTextEncoding::kUTF8, &mb);
        float labelCX = dx + kDlgW * 0.5f;
        float labelCY = navCY;

        // Hover state layer on the label+arrow pill
        if (m_labelHov) {
            SkRect lr = monthYearRect();
            SkRRect lrr; lrr.setRectXY(lr, 18.f, 18.f);
            SkPaint lhp; lhp.setAntiAlias(true); lhp.setColor(withAlpha(th.accent, 20));
            canvas->drawRRect(lrr, lhp);
        }

        SkPaint mp; mp.setAntiAlias(true); mp.setColor(th.textPrimary);
        // Draw text slightly left of center to leave room for the arrow
        const float arrowGap = 14.f;
        float textX = labelCX - (mb.width() + arrowGap) * 0.5f - mb.left();
        canvas->drawString(mv, textX, labelCY - mb.height() * 0.5f - mb.top(), titleSm, mp);

        // Drop arrow (▾ in calendar, ▴ in year-picker)
        float arrowX = textX + mb.width() + arrowGap * 0.5f;
        drawDropArrow(canvas, arrowX, labelCY, m_mode == skDPMode::YearPicker,
                      withAlpha(th.textPrimary, 180));
    }

    // ── CALENDAR MODE ────────────────────────────────────────────────────────
    if (m_mode == skDPMode::Calendar) {
        // Weekday labels
        float wkCY = dy + kHdrH + kNavH + kWkH * 0.5f;
        float gx   = gridX();
        const char* wdLabels[] = {"S","M","T","W","T","F","S"};
        for (int i = 0; i < 7; ++i) {
            SkRect db; bodySm.measureText(wdLabels[i], 1, SkTextEncoding::kUTF8, &db);
            SkPaint dp; dp.setAntiAlias(true); dp.setColor(withAlpha(th.textPrimary, 153));
            canvas->drawString(wdLabels[i],
                gx + i * kCellW + kCellW * 0.5f - db.width() * 0.5f - db.left(),
                wkCY - db.height() * 0.5f - db.top(), bodySm, dp);
        }

        // Day grid
        float gy  = gridY();
        int   fd  = firstWeekday(m_view.year, m_view.month);
        int   dim = daysInMonth(m_view.year, m_view.month);

        for (int day = 1; day <= dim; ++day) {
            int   idx = day - 1 + fd;
            int   col = idx % 7, row = idx / 7;
            float cx  = gx + col * kCellW + kCellW * 0.5f;
            float cy2 = gy + row * kCellH + kCellH * 0.5f;

            bool isSel   = (day == m_sel.day   && m_view.month == m_sel.month   && m_view.year == m_sel.year);
            bool isToday = (day == m_today.day && m_view.month == m_today.month && m_view.year == m_today.year);
            bool isHov   = (day == m_hovDay);

            if (isHov && !isSel) {
                SkPaint hp; hp.setAntiAlias(true); hp.setColor(withAlpha(th.accent, 20));
                canvas->drawCircle(cx, cy2, kSelR, hp);
            }
            if (isSel) {
                SkPaint cp; cp.setAntiAlias(true); cp.setColor(th.accent);
                canvas->drawCircle(cx, cy2, kSelR, cp);
            } else if (isToday) {
                SkPaint cp; cp.setAntiAlias(true);
                cp.setStyle(SkPaint::kStroke_Style); cp.setStrokeWidth(1.5f);
                cp.setColor(th.accent);
                canvas->drawCircle(cx, cy2, kSelR - 0.75f, cp);
            }

            char d2[4]; snprintf(d2, sizeof(d2), "%d", day);
            SkRect db; bodyMed.measureText(d2, strlen(d2), SkTextEncoding::kUTF8, &db);
            SkPaint dp; dp.setAntiAlias(true);
            dp.setColor(isSel ? th.textOnAccent : (isToday ? th.accent : th.textPrimary));
            canvas->drawString(d2,
                cx - db.width() * 0.5f - db.left(),
                cy2 - db.height() * 0.5f - db.top(),
                bodyMed, dp);
        }
    }

    // ── YEAR-PICKER MODE ─────────────────────────────────────────────────────
    else {
        float gx = yearGridX(), gy = yearGridY();

        for (int i = 0; i < kYearsPerPg; ++i) {
            int   year = m_yearPage + i;
            int   col  = i % kYearCols;
            int   row  = i / kYearCols;
            float cx   = gx + col * kYearCellW + kYearCellW * 0.5f;
            float cy2  = gy + row * kYearCellH + kYearCellH * 0.5f;

            bool isSel  = (year == m_view.year);
            bool isCur  = (year == m_today.year);
            bool isHov  = (year == m_hovYear);

            // Pill shape for year cell
            float pillW = kYearCellW * 0.78f, pillH = kYearCellH * 0.62f;
            SkRect pill = SkRect::MakeXYWH(cx - pillW * 0.5f, cy2 - pillH * 0.5f, pillW, pillH);
            SkRRect pillRR; pillRR.setRectXY(pill, pillH * 0.5f, pillH * 0.5f);

            if (isSel) {
                SkPaint fp; fp.setAntiAlias(true); fp.setColor(th.accent);
                canvas->drawRRect(pillRR, fp);
            } else if (isHov) {
                SkPaint hp; hp.setAntiAlias(true); hp.setColor(withAlpha(th.accent, 20));
                canvas->drawRRect(pillRR, hp);
            }
            if (isCur && !isSel) {
                SkPaint op; op.setAntiAlias(true);
                op.setStyle(SkPaint::kStroke_Style); op.setStrokeWidth(1.5f);
                op.setColor(th.accent);
                canvas->drawRRect(pillRR, op);
            }

            char ybuf[8]; snprintf(ybuf, sizeof(ybuf), "%d", year);
            SkRect yb; bodyMed.measureText(ybuf, strlen(ybuf), SkTextEncoding::kUTF8, &yb);
            SkPaint yp; yp.setAntiAlias(true);
            yp.setColor(isSel ? th.textOnAccent : (isCur ? th.accent : th.textPrimary));
            canvas->drawString(ybuf,
                cx - yb.width() * 0.5f - yb.left(),
                cy2 - yb.height() * 0.5f - yb.top(),
                bodyMed, yp);
        }
    }

    // ── ACTION BAR ───────────────────────────────────────────────────────────
    float actY = dy + kDlgH - kActH;
    {
        SkPaint div; div.setAntiAlias(true);
        div.setStyle(SkPaint::kStroke_Style); div.setStrokeWidth(1.f);
        div.setColor(withAlpha(th.textPrimary, 30));
        canvas->drawLine(dx, actY, dx + kDlgW, actY, div);
    }
    auto drawTextBtn = [&](SkRect r, const char* lbl, bool hov) {
        if (hov) {
            SkRRect hr; hr.setRectXY(r, 18.f, 18.f);
            SkPaint hp; hp.setAntiAlias(true); hp.setColor(withAlpha(th.accent, 20));
            canvas->drawRRect(hr, hp);
        }
        SkRect lb; lblLarge.measureText(lbl, strlen(lbl), SkTextEncoding::kUTF8, &lb);
        SkPaint lp; lp.setAntiAlias(true); lp.setColor(th.accent);
        canvas->drawString(lbl,
            r.centerX() - lb.width() * 0.5f - lb.left(),
            r.centerY() - lb.height() * 0.5f - lb.top(),
            lblLarge, lp);
    };
    drawTextBtn(cancelRect(), "Cancel", m_cancelHov);
    drawTextBtn(okRect(),     "OK",     m_okHov);

    canvas->restore();
}

bool skDatePicker::handleEvent(const skEvent& ev) {
    if (!visible()) return false;

    if (ev.type == skEventType::KeyDown) {
        if (ev.button == VK_ESCAPE) {
            if (m_mode == skDPMode::YearPicker) {
                // Escape from year picker returns to calendar, doesn't dismiss
                m_mode = skDPMode::Calendar;
                m_hovYear = 0;
            } else {
                setVisible(false);
                if (m_onCancel) m_onCancel();
            }
            return true;
        }
    }

    SkRect okR = okRect(), canR = cancelRect(), prR = prevRect(), nxR = nextRect();
    SkRect lblR = monthYearRect();

    if (ev.type == skEventType::MouseMove) {
        m_okHov     = okR.contains((float)ev.x, (float)ev.y);
        m_cancelHov = canR.contains((float)ev.x, (float)ev.y);
        m_prevHov   = prR.contains((float)ev.x, (float)ev.y);
        m_nextHov   = nxR.contains((float)ev.x, (float)ev.y);
        m_labelHov  = lblR.contains((float)ev.x, (float)ev.y);
        m_hovDay    = (m_mode == skDPMode::Calendar)   ? dayAt(ev.x, ev.y)  : 0;
        m_hovYear   = (m_mode == skDPMode::YearPicker) ? yearAt(ev.x, ev.y) : 0;
        return true;
    }

    if (ev.type == skEventType::MouseDown) {
        // Nav chevrons (behavior differs per mode)
        if (prR.contains((float)ev.x, (float)ev.y)) {
            prevPage(); m_hovDay = m_hovYear = 0; return true;
        }
        if (nxR.contains((float)ev.x, (float)ev.y)) {
            nextPage(); m_hovDay = m_hovYear = 0; return true;
        }

        // Month/year label toggles between calendar and year-picker
        if (lblR.contains((float)ev.x, (float)ev.y)) {
            if (m_mode == skDPMode::Calendar) {
                m_mode = skDPMode::YearPicker;
                m_yearPage = (m_view.year / kYearsPerPg) * kYearsPerPg;
                m_hovDay   = 0;
            } else {
                m_mode    = skDPMode::Calendar;
                m_hovYear = 0;
            }
            return true;
        }

        if (m_mode == skDPMode::Calendar) {
            int d = dayAt(ev.x, ev.y);
            if (d > 0) {
                m_sel = { m_view.year, m_view.month, d };
                return true;
            }
        } else {
            // Year picker: clicking a year selects it and returns to calendar
            int yr = yearAt(ev.x, ev.y);
            if (yr != 0) {
                m_view.year = yr;
                m_sel.year  = yr;
                // Clamp day in case month has fewer days in the new year (e.g. Feb 29)
                int dim = daysInMonth(m_view.year, m_view.month);
                if (m_sel.day > dim) m_sel.day = m_view.day = dim;
                m_mode    = skDPMode::Calendar;
                m_hovYear = 0;
                return true;
            }
        }

        if (okR.contains((float)ev.x, (float)ev.y)) {
            setVisible(false); if (m_onConfirm) m_onConfirm(m_sel); return true;
        }
        if (canR.contains((float)ev.x, (float)ev.y)) {
            setVisible(false); if (m_onCancel) m_onCancel(); return true;
        }

        // Click outside dialog → dismiss
        float ddx = dlgX(), ddy = dlgY();
        bool inDlg = ((float)ev.x >= ddx && (float)ev.x < ddx + kDlgW &&
                      (float)ev.y >= ddy && (float)ev.y < ddy + kDlgH);
        if (!inDlg) { setVisible(false); if (m_onCancel) m_onCancel(); }
        return true;
    }
    return true;
}
