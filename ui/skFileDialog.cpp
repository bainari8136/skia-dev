#include <Windows.h>
#include "skFileDialog.h"
#include "skTheme.h"
#include "skTypeface.h"
#include <include/core/SkPaint.h>
#include <include/core/SkFont.h>
#include <include/core/SkPath.h>
#include <include/core/SkRRect.h>
#include <algorithm>
#include <cstring>
#include <cstdio>

static sk_sp<SkTypeface>& fdTf() {
    static sk_sp<SkTypeface> tf = skGetSystemTypeface();
    return tf;
}

static SkColor withAlpha(SkColor c, uint8_t a) {
    return SkColorSetARGB(a, SkColorGetR(c), SkColorGetG(c), SkColorGetB(c));
}

// ── Geometry ─────────────────────────────────────────────────────────────────

SkRect skFileDialog::upRect() const {
    float cy = dlgY() + kHdrH + kPathH * 0.5f;
    return SkRect::MakeXYWH(dlgX() + 8.f, cy - 20.f, 40.f, 40.f);
}
SkRect skFileDialog::listRect() const {
    return SkRect::MakeXYWH(dlgX(), dlgY() + kHdrH + kPathH, (float)kDlgW, kListH);
}
SkRect skFileDialog::previewRect() const {
    return SkRect::MakeXYWH(dlgX(), dlgY() + kHdrH + kPathH + kListH, (float)kDlgW, kPreviewH);
}
SkRect skFileDialog::okRect() const {
    float actY = dlgY() + kDlgH - kActH;
    return SkRect::MakeXYWH(dlgX() + kDlgW - 84.f, actY + (kActH - 36.f) * 0.5f, 72.f, 36.f);
}
SkRect skFileDialog::cancelRect() const {
    SkRect ok = okRect();
    return SkRect::MakeXYWH(ok.left() - 88.f, ok.top(), 76.f, ok.height());
}

int skFileDialog::visibleRows() const { return std::max(1, (int)(kListH / kRowH)); }
int skFileDialog::maxScroll()   const { return std::max(0, (int)m_entries.size() - visibleRows()); }

int skFileDialog::entryAt(int py) const {
    SkRect lr = listRect();
    if (py < (int)lr.top() || py >= (int)lr.bottom()) return -1;
    int idx = (int)((float)(py - (int)lr.top()) / kRowH) + m_scroll;
    return (idx >= 0 && idx < (int)m_entries.size()) ? idx : -1;
}

SkRect skFileDialog::sbThumbRect() const {
    int ms = maxScroll();
    if (ms <= 0 || m_entries.empty()) return SkRect::MakeEmpty();
    SkRect lr  = listRect();
    int    vis = visibleRows();
    float sbX    = lr.right() - kSbW - 2.f;
    float ratio  = (float)vis / (float)m_entries.size();
    float thumbH = std::max(24.f, lr.height() * ratio);
    float thumbY = lr.top() + (float)m_scroll / (float)ms * (lr.height() - thumbH);
    return SkRect::MakeXYWH(sbX, thumbY + 2.f, kSbW, thumbH - 4.f);
}

// ── Constructor ───────────────────────────────────────────────────────────────

skFileDialog::skFileDialog(int winW, int winH) : skWidget(0, 0, winW, winH) {
    setVisible(false);
}

// ── Directory logic ───────────────────────────────────────────────────────────

void skFileDialog::loadDirectory(const std::string& path) {
    m_entries.clear();
    m_selected = m_lastClickIdx = -1;
    m_scroll = 0;
    m_hovEntry = -1;

    std::string pattern = path;
    if (!pattern.empty() && pattern.back() != '\\') pattern += '\\';
    pattern += '*';

    WIN32_FIND_DATAA fd;
    HANDLE h = FindFirstFileA(pattern.c_str(), &fd);
    if (h == INVALID_HANDLE_VALUE) return;
    do {
        std::string name(fd.cFileName);
        if (name == ".") continue;
        bool isDir = (fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0;
        m_entries.push_back({name, isDir});
    } while (FindNextFileA(h, &fd));
    FindClose(h);

    std::sort(m_entries.begin(), m_entries.end(), [](const Entry& a, const Entry& b) {
        if (a.name == "..") return true;
        if (b.name == "..") return false;
        if (a.isDir != b.isDir) return a.isDir > b.isDir;
        return a.name < b.name;
    });

    m_currentPath = path;
    if (!m_currentPath.empty() && m_currentPath.back() == '\\')
        m_currentPath.pop_back();
    m_selectedPath.clear();
}

void skFileDialog::navigateUp() {
    size_t pos = m_currentPath.rfind('\\');
    if (pos == std::string::npos) return;
    std::string parent = m_currentPath.substr(0, pos);
    if (parent.empty()) parent = m_currentPath.substr(0, pos + 1);
    loadDirectory(parent);
}

void skFileDialog::openEntry(int idx) {
    if (idx < 0 || idx >= (int)m_entries.size()) return;
    const auto& e = m_entries[idx];
    if (e.name == "..") { navigateUp(); return; }
    if (e.isDir) {
        loadDirectory(m_currentPath + '\\' + e.name);
    } else {
        m_selectedPath = m_currentPath + '\\' + e.name;
        m_selected = idx;
    }
}

void skFileDialog::show(const std::string& startPath) {
    loadDirectory(startPath);
    m_okHov = m_cancelHov = m_upHov = false;
    setVisible(true);
}

// ── Icon helpers ──────────────────────────────────────────────────────────────

void skFileDialog::drawUpChevron(SkCanvas* canvas, float cx, float cy, SkColor color) const {
    const float s = 5.5f;
    SkPath path;
    path.moveTo(cx - s * 1.2f, cy + s);
    path.lineTo(cx, cy - s);
    path.lineTo(cx + s * 1.2f, cy + s);
    SkPaint p;
    p.setAntiAlias(true);
    p.setStyle(SkPaint::kStroke_Style);
    p.setStrokeWidth(2.f);
    p.setStrokeCap(SkPaint::kRound_Cap);
    p.setStrokeJoin(SkPaint::kRound_Join);
    p.setColor(color);
    canvas->drawPath(path, p);
}

// ── Paint ─────────────────────────────────────────────────────────────────────

void skFileDialog::Paint(SkCanvas* canvas) {
    const auto& th = skGetTheme();
    float dx = dlgX(), dy = dlgY();

    // Scrim
    SkPaint scrimP; scrimP.setColor(SkColorSetARGB(130, 0, 0, 0));
    canvas->drawRect(SkRect::MakeXYWH((float)x, (float)y, (float)w, (float)h), scrimP);

    // Layered shadow
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

    // ── HEADER ───────────────────────────────────────────────────────────────
    SkFont titleLg (fdTf(), 22.f); titleLg.setEdging(SkFont::Edging::kAntiAlias);
    SkFont bodyMed (fdTf(), 14.f); bodyMed.setEdging(SkFont::Edging::kAntiAlias);
    SkFont bodySm  (fdTf(), 13.f); bodySm.setEdging(SkFont::Edging::kAntiAlias);
    SkFont lblLarge(fdTf(), 14.f); lblLarge.setEdging(SkFont::Edging::kAntiAlias);

    {
        const char* title = "Open File";
        SkRect tb; titleLg.measureText(title, strlen(title), SkTextEncoding::kUTF8, &tb);
        SkPaint tp; tp.setAntiAlias(true); tp.setColor(th.textPrimary);
        float hdrCY = dy + kHdrH * 0.5f;
        canvas->drawString(title, dx + 24.f, hdrCY - tb.height() * 0.5f - tb.top(), titleLg, tp);
    }
    // Header / path-bar divider
    {
        SkPaint div; div.setAntiAlias(true);
        div.setStyle(SkPaint::kStroke_Style); div.setStrokeWidth(1.f);
        div.setColor(withAlpha(th.textPrimary, 25));
        canvas->drawLine(dx, dy + kHdrH, dx + kDlgW, dy + kHdrH, div);
    }

    // ── PATH BAR ─────────────────────────────────────────────────────────────
    // Subtle tinted background (surfaceContainerLow feel)
    {
        SkPaint pbg; pbg.setColor(withAlpha(th.textPrimary, 10));
        canvas->drawRect(SkRect::MakeXYWH(dx, dy + kHdrH, (float)kDlgW, kPathH), pbg);
    }

    // Up chevron button (40×40dp touch target, circular hover)
    {
        SkRect ur = upRect();
        if (m_upHov) {
            SkPaint hp; hp.setAntiAlias(true); hp.setColor(withAlpha(th.accent, 20));
            canvas->drawCircle(ur.centerX(), ur.centerY(), 20.f, hp);
        }
        SkColor chCol = m_upHov ? th.accent : withAlpha(th.textPrimary, 160);
        drawUpChevron(canvas, ur.centerX(), ur.centerY(), chCol);
    }

    // Current path text — truncate from left if long
    {
        std::string display = m_currentPath;
        // Reserve space after up button (56dp) and right padding (16dp)
        const size_t maxChars = 48;
        if (display.size() > maxChars)
            display = "\xe2\x80\xa6" + display.substr(display.size() - (maxChars - 1)); // "…"
        SkRect pb; bodyMed.measureText(display.c_str(), display.size(), SkTextEncoding::kUTF8, &pb);
        SkPaint pp; pp.setAntiAlias(true); pp.setColor(withAlpha(th.textPrimary, 180));
        float pathCY = dy + kHdrH + kPathH * 0.5f;
        canvas->drawString(display.c_str(), dx + 56.f, pathCY - pb.height() * 0.5f - pb.top(), bodyMed, pp);
    }

    // Path bar / list divider
    {
        SkPaint div; div.setAntiAlias(true);
        div.setStyle(SkPaint::kStroke_Style); div.setStrokeWidth(1.f);
        div.setColor(withAlpha(th.textPrimary, 25));
        canvas->drawLine(dx, dy + kHdrH + kPathH, dx + kDlgW, dy + kHdrH + kPathH, div);
    }

    // ── FILE LIST ─────────────────────────────────────────────────────────────
    SkRect lr = listRect();
    canvas->save();
    canvas->clipRect(lr);

    // Inline icon-drawing lambdas (M3 Material Icons approximations)
    auto drawFolder = [&](float cx, float cy, SkColor col) {
        SkPaint p; p.setAntiAlias(true); p.setColor(col);
        // tab (rounded nub at top-left of folder)
        SkRRect tab; tab.setRectXY(SkRect::MakeXYWH(cx - 11.f, cy - 10.f, 9.f, 5.f), 1.5f, 1.5f);
        canvas->drawRRect(tab, p);
        // body
        SkRRect body; body.setRectXY(SkRect::MakeXYWH(cx - 11.f, cy - 7.f, 22.f, 16.f), 2.f, 2.f);
        canvas->drawRRect(body, p);
    };

    auto drawFile = [&](float cx, float cy, SkColor col) {
        const float fw = 9.f, fh = 11.f, fold = 4.f;
        SkPath path;
        path.moveTo(cx - fw, cy - fh);
        path.lineTo(cx + fw - fold, cy - fh);
        path.lineTo(cx + fw, cy - fh + fold);
        path.lineTo(cx + fw, cy + fh);
        path.lineTo(cx - fw, cy + fh);
        path.close();
        SkPaint fp; fp.setAntiAlias(true); fp.setColor(col);
        canvas->drawPath(path, fp);
        // Dog-ear crease
        SkPath crease;
        crease.moveTo(cx + fw - fold, cy - fh);
        crease.lineTo(cx + fw - fold, cy - fh + fold);
        crease.lineTo(cx + fw, cy - fh + fold);
        SkPaint cp; cp.setAntiAlias(true);
        cp.setStyle(SkPaint::kStroke_Style); cp.setStrokeWidth(1.f);
        cp.setColor(withAlpha(th.textPrimary, 50));
        canvas->drawPath(crease, cp);
    };

    int vis  = visibleRows();
    int eend = std::min((int)m_entries.size(), m_scroll + vis);

    for (int i = m_scroll; i < eend; ++i) {
        float ry  = lr.top() + (float)(i - m_scroll) * kRowH;
        float rcy = ry + kRowH * 0.5f;
        bool isSel = (i == m_selected);
        bool isHov = (i == m_hovEntry);

        // State layer: selected (12% primary) or hover (8% onSurface)
        if (isSel) {
            SkPaint sp; sp.setColor(withAlpha(th.accent, 30));
            canvas->drawRect(SkRect::MakeXYWH(lr.left(), ry, lr.width(), kRowH), sp);
        } else if (isHov) {
            SkPaint hp; hp.setColor(withAlpha(th.textPrimary, 20));
            canvas->drawRect(SkRect::MakeXYWH(lr.left(), ry, lr.width(), kRowH), hp);
        }

        // Icon (centered at x=dx+28, row vertical center)
        float iconX = dx + 28.f;
        if (m_entries[i].isDir) {
            drawFolder(iconX, rcy, withAlpha(th.accent, 200));
        } else {
            drawFile(iconX, rcy, withAlpha(th.textPrimary, 100));
        }

        // Name — bodyLarge (16sp equivalent), truncated
        std::string name = m_entries[i].name;
        if (name.size() > 54) name = name.substr(0, 52) + "\xe2\x80\xa6";
        SkPaint np; np.setAntiAlias(true);
        np.setColor(isSel ? th.accent : th.textPrimary);
        SkRect nb; bodyMed.measureText(name.c_str(), name.size(), SkTextEncoding::kUTF8, &nb);
        canvas->drawString(name.c_str(), dx + 56.f, rcy - nb.height() * 0.5f - nb.top(), bodyMed, np);
    }

    // M3 scrollbar: thin, rounded, subtle
    {
        int ms = maxScroll();
        if (ms > 0) {
            float sbX    = lr.right() - kSbW - 2.f;
            float ratio  = (float)vis / (float)m_entries.size();
            float thumbH = std::max(24.f, lr.height() * ratio);
            float thumbY = lr.top() + (float)m_scroll / (float)ms * (lr.height() - thumbH);
            // Track
            SkPaint trk; trk.setAntiAlias(true); trk.setColor(withAlpha(th.textPrimary, 12));
            canvas->drawRoundRect(SkRect::MakeXYWH(sbX, lr.top(), kSbW, lr.height()), kSbW * 0.5f, kSbW * 0.5f, trk);
            // Thumb
            SkPaint tP; tP.setAntiAlias(true); tP.setColor(withAlpha(th.textPrimary, 80));
            canvas->drawRoundRect(SkRect::MakeXYWH(sbX, thumbY + 2.f, kSbW, thumbH - 4.f), kSbW * 0.5f, kSbW * 0.5f, tP);
        }
    }

    canvas->restore(); // end list clip

    // ── PREVIEW STRIP ────────────────────────────────────────────────────────
    SkRect pr = previewRect();
    {
        SkPaint div; div.setAntiAlias(true);
        div.setStyle(SkPaint::kStroke_Style); div.setStrokeWidth(1.f);
        div.setColor(withAlpha(th.textPrimary, 25));
        canvas->drawLine(dx, pr.top(), dx + kDlgW, pr.top(), div);
    }
    // Subtle background (surfaceContainerLow)
    {
        SkPaint pbg; pbg.setColor(withAlpha(th.textPrimary, 8));
        canvas->drawRect(pr, pbg);
    }
    {
        std::string preview = m_selectedPath.empty() ? "No file selected" : m_selectedPath;
        if (preview.size() > 58) preview = "\xe2\x80\xa6" + preview.substr(preview.size() - 56);
        SkPaint pp; pp.setAntiAlias(true);
        pp.setColor(m_selectedPath.empty() ? withAlpha(th.textPrimary, 100) : th.textPrimary);
        SkRect pb; bodySm.measureText(preview.c_str(), preview.size(), SkTextEncoding::kUTF8, &pb);
        canvas->drawString(preview.c_str(), dx + 24.f, pr.centerY() - pb.height() * 0.5f - pb.top(), bodySm, pp);
    }

    // ── ACTION BAR ───────────────────────────────────────────────────────────
    float actY = dy + kDlgH - kActH;
    {
        SkPaint div; div.setAntiAlias(true);
        div.setStyle(SkPaint::kStroke_Style); div.setStrokeWidth(1.f);
        div.setColor(withAlpha(th.textPrimary, 25));
        canvas->drawLine(dx, actY, dx + kDlgW, actY, div);
    }

    // M3 text buttons: accent label, 8% state layer on hover
    auto drawTextBtn = [&](SkRect r, const char* lbl, bool hov, bool enabled) {
        if (hov && enabled) {
            SkRRect hr; hr.setRectXY(r, 18.f, 18.f);
            SkPaint hp; hp.setAntiAlias(true); hp.setColor(withAlpha(th.accent, 20));
            canvas->drawRRect(hr, hp);
        }
        SkRect lb; lblLarge.measureText(lbl, strlen(lbl), SkTextEncoding::kUTF8, &lb);
        SkPaint lp; lp.setAntiAlias(true);
        lp.setColor(enabled ? th.accent : withAlpha(th.textPrimary, 60));
        canvas->drawString(lbl,
            r.centerX() - lb.width() * 0.5f - lb.left(),
            r.centerY() - lb.height() * 0.5f - lb.top(),
            lblLarge, lp);
    };

    bool canOpen = !m_selectedPath.empty();
    drawTextBtn(cancelRect(), "Cancel", m_cancelHov, true);
    drawTextBtn(okRect(),     "Open",   m_okHov,     canOpen);

    canvas->restore(); // end dialog clip
}

// ── Event handling ────────────────────────────────────────────────────────────

bool skFileDialog::handleEvent(const skEvent& ev) {
    if (!visible()) return false;

    if (ev.type == skEventType::KeyDown && ev.button == VK_ESCAPE) {
        setVisible(false); if (m_onCancel) m_onCancel(); return true;
    }

    // Release scrollbar drag on any mouse-up or cancel
    if (ev.type == skEventType::MouseUp || ev.type == skEventType::MouseCancel) {
        m_sbDragging = false;
        return true;
    }

    SkRect lr = listRect(), okR = okRect(), canR = cancelRect(), ur = upRect();

    if (ev.type == skEventType::MouseMove) {
        if (m_sbDragging) {
            float delta = (float)ev.y - m_sbDragStartY;
            int   ms    = maxScroll();
            if (ms > 0) {
                int   vis    = visibleRows();
                float ratio  = (float)vis / (float)m_entries.size();
                float thumbH = std::max(24.f, lr.height() * ratio);
                float trackH = lr.height() - thumbH;
                if (trackH > 0.f)
                    m_scroll = std::clamp(
                        m_sbDragStartScroll + (int)std::round(delta / trackH * (float)ms),
                        0, ms);
            }
            return true;
        }
        m_okHov     = okR.contains((float)ev.x, (float)ev.y);
        m_cancelHov = canR.contains((float)ev.x, (float)ev.y);
        m_upHov     = ur.contains((float)ev.x, (float)ev.y);
        m_hovEntry  = lr.contains((float)ev.x, (float)ev.y) ? entryAt(ev.y) : -1;
        return true;
    }

    if (ev.type == skEventType::MouseWheel && lr.contains((float)ev.x, (float)ev.y)) {
        int ms = maxScroll();
        m_scroll = (ev.button > 0) ? std::max(0, m_scroll - 2) : std::min(ms, m_scroll + 2);
        return true;
    }

    if (ev.type == skEventType::MouseDown) {
        // Scrollbar: thumb drag or track page-jump
        int ms = maxScroll();
        if (ms > 0) {
            SkRect thumb = sbThumbRect();
            // Widen hit area slightly to the left so the thin thumb is easier to grab
            SkRect sbArea = SkRect::MakeXYWH(thumb.left() - 4.f, lr.top(),
                                              thumb.width() + 8.f, lr.height());
            if (sbArea.contains((float)ev.x, (float)ev.y)) {
                if (thumb.contains((float)ev.x, (float)ev.y)) {
                    m_sbDragging       = true;
                    m_sbDragStartY     = (float)ev.y;
                    m_sbDragStartScroll = m_scroll;
                } else {
                    // Track click: page up or page down
                    if ((float)ev.y < thumb.top())
                        m_scroll = std::max(0,  m_scroll - visibleRows());
                    else
                        m_scroll = std::min(ms, m_scroll + visibleRows());
                }
                return true;
            }
        }

        if (ur.contains((float)ev.x, (float)ev.y))  { navigateUp(); return true; }
        if (okR.contains((float)ev.x, (float)ev.y)) {
            if (!m_selectedPath.empty()) {
                setVisible(false); if (m_onConfirm) m_onConfirm(m_selectedPath);
            }
            return true;
        }
        if (canR.contains((float)ev.x, (float)ev.y)) {
            setVisible(false); if (m_onCancel) m_onCancel(); return true;
        }
        if (lr.contains((float)ev.x, (float)ev.y)) {
            int idx = entryAt(ev.y);
            if (idx >= 0) {
                if (idx == m_lastClickIdx) {
                    openEntry(idx);
                } else {
                    m_selected = idx;
                    if (!m_entries[idx].isDir)
                        m_selectedPath = m_currentPath + '\\' + m_entries[idx].name;
                    else
                        m_selectedPath.clear();
                }
                m_lastClickIdx = idx;
            }
            return true;
        }
        // Click outside → dismiss
        float ddx = dlgX(), ddy = dlgY();
        bool inDlg = ((float)ev.x >= ddx && (float)ev.x < ddx + kDlgW &&
                      (float)ev.y >= ddy && (float)ev.y < ddy + kDlgH);
        if (!inDlg) { setVisible(false); if (m_onCancel) m_onCancel(); }
        return true;
    }
    return true;
}
