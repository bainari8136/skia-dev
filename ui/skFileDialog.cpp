#include <Windows.h>
#include "skFileDialog.h"
#include "skTheme.h"
#include "skTypeface.h"
#include <include/core/SkPaint.h>
#include <include/core/SkFont.h>
#include <include/core/SkRRect.h>
#include <algorithm>
#include <cstring>
#include <cstdio>

static sk_sp<SkTypeface>& fdTf() {
    static sk_sp<SkTypeface> tf = skGetSystemTypeface();
    return tf;
}

skFileDialog::skFileDialog(int winW, int winH)
    : skWidget(0, 0, winW, winH) { setVisible(false); }

// ---- Geometry helpers -------------------------------------------------
SkRect skFileDialog::upRect() const {
    return SkRect::MakeXYWH(dlgX() + 8.f, dlgY() + (kToolH-26.f)/2.f, 26.f, 26.f);
}
SkRect skFileDialog::listRect() const {
    return SkRect::MakeXYWH(dlgX() + 8.f, dlgY() + kListY, (float)kDlgW - 16.f, kListH);
}
SkRect skFileDialog::okRect() const {
    return SkRect::MakeXYWH(dlgX() + kDlgW - kBtnW - 10.f,
                             dlgY() + kDlgH - kBtnH - 10.f, kBtnW, kBtnH);
}
SkRect skFileDialog::cancelRect() const {
    SkRect ok = okRect();
    return SkRect::MakeXYWH(ok.left() - kBtnW - 8.f, ok.top(), kBtnW, kBtnH);
}

int skFileDialog::visibleRows() const {
    return std::max(1, (int)(kListH / kRowH));
}
int skFileDialog::maxScroll() const {
    return std::max(0, (int)m_entries.size() - visibleRows());
}
int skFileDialog::entryAt(int py) const {
    SkRect lr = listRect();
    if (py < (int)lr.top() || py >= (int)lr.bottom()) return -1;
    int idx = (int)((float)(py - (int)lr.top()) / kRowH) + m_scroll;
    return (idx >= 0 && idx < (int)m_entries.size()) ? idx : -1;
}

// ---- Directory listing ------------------------------------------------
void skFileDialog::loadDirectory(const std::string& path) {
    m_entries.clear();
    m_selected = m_lastClickIdx = -1;
    m_scroll = 0;

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

    // Sort: ".." first, then dirs alphabetically, then files
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
    if (parent.empty()) parent = m_currentPath.substr(0, pos+1); // drive root
    loadDirectory(parent);
}

void skFileDialog::openEntry(int idx) {
    if (idx < 0 || idx >= (int)m_entries.size()) return;
    auto& e = m_entries[idx];
    if (e.name == "..") { navigateUp(); return; }
    if (e.isDir) {
        std::string next = m_currentPath + '\\' + e.name;
        loadDirectory(next);
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

// ---- Paint ------------------------------------------------------------
void skFileDialog::Paint(SkCanvas* canvas) {
    const auto& th = skGetTheme();
    float dx = dlgX(), dy = dlgY();

    // Backdrop
    SkPaint bk; bk.setColor(SkColorSetARGB(130,0,0,0));
    canvas->drawRect(SkRect::MakeXYWH((float)x,(float)y,(float)w,(float)h), bk);

    // Shadow + dialog background
    SkRRect sh; sh.setRectXY(SkRect::MakeXYWH(dx+2.f,dy+5.f,(float)kDlgW,(float)kDlgH),kR,kR);
    SkPaint shP; shP.setAntiAlias(true); shP.setColor(SkColorSetARGB(28,0,0,0));
    canvas->drawRRect(sh, shP);

    SkRRect rr; rr.setRectXY(SkRect::MakeXYWH(dx,dy,(float)kDlgW,(float)kDlgH),kR,kR);
    SkPaint bgP; bgP.setAntiAlias(true); bgP.setColor(th.panelBg);
    canvas->drawRRect(rr, bgP);

    canvas->save();
    canvas->clipRRect(rr, SkClipOp::kIntersect, true);
    SkPaint bar; bar.setColor(th.accent);
    canvas->drawRect(SkRect::MakeXYWH(dx,dy,(float)kDlgW,5.f), bar);
    canvas->restore();

    SkPaint brd; brd.setAntiAlias(true);
    brd.setStyle(SkPaint::kStroke_Style); brd.setStrokeWidth(0.75f);
    brd.setColor(th.panelBorder);
    canvas->drawRRect(rr, brd);

    SkFont font(fdTf(), 12.f);  font.setEdging(SkFont::Edging::kAntiAlias);
    SkFont small(fdTf(), 11.f); small.setEdging(SkFont::Edging::kAntiAlias);

    // Up button
    {
        SkRect ur = upRect();
        SkRRect urr; urr.setRectXY(ur, 4.f, 4.f);
        SkPaint up; up.setAntiAlias(true);
        up.setColor(m_upHov ? th.accentHover : SkColorSetARGB(0,0,0,0));
        canvas->drawRRect(urr, up);
        SkPaint uc; uc.setAntiAlias(true); uc.setColor(m_upHov ? th.accent : th.textSecondary);
        canvas->drawString("\xe2\x86\x91", ur.centerX()-5.f, ur.centerY()+5.f, font, uc); // ↑
    }

    // Current path label
    {
        std::string display = m_currentPath.size() > 40 ? "..." + m_currentPath.substr(m_currentPath.size()-38) : m_currentPath;
        SkPaint pp; pp.setAntiAlias(true); pp.setColor(th.textPrimary);
        canvas->drawString(display.c_str(), dx + 38.f, dy + kToolH/2.f + 5.f, small, pp);
    }

    // Path bar separator
    SkPaint sep; sep.setColor(th.panelBorder);
    canvas->drawRect(SkRect::MakeXYWH(dx+8.f, dy+kToolH, (float)kDlgW-16.f, 1.f), sep);

    // File list
    SkRect lr = listRect();
    canvas->save();
    canvas->clipRect(lr);
    int vis  = visibleRows();
    int eend = std::min((int)m_entries.size(), m_scroll + vis);
    for (int i = m_scroll; i < eend; ++i) {
        float ry = lr.top() + (float)(i - m_scroll) * kRowH;
        bool sel = (i == m_selected);
        if (sel) {
            SkPaint sp; sp.setColor(SkColorSetARGB(40, SkColorGetR(th.accent),
                SkColorGetG(th.accent), SkColorGetB(th.accent)));
            canvas->drawRect(SkRect::MakeXYWH(lr.left(), ry, lr.width()-kSbW, kRowH), sp);
        }
        // Icon: folder = "📁" UTF-8, file = "📄" (or just use text markers)
        const char* icon = m_entries[i].isDir ? "\xf0\x9f\x93\x81" : "\xf0\x9f\x93\x84";
        // Use simple ASCII alternatives for reliability
        const char* icon2 = m_entries[i].isDir ? "[D]" : "[F]";
        SkPaint ip; ip.setAntiAlias(true);
        ip.setColor(m_entries[i].isDir ? th.accent : th.textSecondary);
        canvas->drawString(icon2, lr.left()+2.f, ry+kRowH-5.f, small, ip);

        SkPaint np; np.setAntiAlias(true);
        np.setColor(sel ? th.textPrimary : th.textPrimary);
        canvas->drawString(m_entries[i].name.c_str(), lr.left()+28.f, ry+kRowH-5.f, small, np);
    }
    canvas->restore();

    // List border
    SkPaint lbrd; lbrd.setAntiAlias(true);
    lbrd.setStyle(SkPaint::kStroke_Style); lbrd.setStrokeWidth(1.f);
    lbrd.setColor(th.inputBorder);
    canvas->drawRect(lr, lbrd);

    // Vertical scrollbar
    int ms = maxScroll();
    if (ms > 0) {
        float sbX = lr.right() - kSbW;
        SkPaint trk; trk.setColor(SkColorSetARGB(30,128,128,128));
        canvas->drawRect(SkRect::MakeXYWH(sbX, lr.top(), kSbW, lr.height()), trk);
        float ratio  = (float)vis / (float)(vis + ms);
        float thumbH = std::max(20.f, lr.height() * ratio);
        float thumbY = lr.top() + (float)m_scroll/(float)ms * (lr.height()-thumbH);
        SkRRect tRR; tRR.setRectXY(SkRect::MakeXYWH(sbX+1.f,thumbY+1.f,kSbW-2.f,thumbH-2.f),3.f,3.f);
        SkPaint tP; tP.setAntiAlias(true); tP.setColor(th.inputBorder);
        canvas->drawRRect(tRR, tP);
    }

    // Selected file path preview
    {
        SkPaint pp; pp.setAntiAlias(true); pp.setColor(th.textSecondary);
        std::string preview = m_selectedPath.empty() ? "No file selected" : m_selectedPath;
        if (preview.size() > 50) preview = "..." + preview.substr(preview.size()-48);
        canvas->drawString(preview.c_str(), dx+10.f, okRect().top()-12.f, small, pp);
    }

    // OK / Cancel
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
            r.centerX()-lb.width()/2.f-lb.left(),
            r.centerY()-lb.height()/2.f-lb.top(), small, lp);
    };
    drawBtn(okRect(),     "Open",   true,  m_okHov);
    drawBtn(cancelRect(), "Cancel", false, m_cancelHov);
}

// ---- Event handling ---------------------------------------------------
bool skFileDialog::handleEvent(const skEvent& ev) {
    if (!visible()) return false;

    if (ev.type == skEventType::KeyDown && ev.button == VK_ESCAPE) {
        setVisible(false); if (m_onCancel) m_onCancel(); return true;
    }

    SkRect lr = listRect(), okR = okRect(), canR = cancelRect(), ur = upRect();

    if (ev.type == skEventType::MouseMove) {
        m_okHov     = okR.contains((float)ev.x,(float)ev.y);
        m_cancelHov = canR.contains((float)ev.x,(float)ev.y);
        m_upHov     = ur.contains((float)ev.x,(float)ev.y);
        return true;
    }

    if (ev.type == skEventType::MouseWheel && lr.contains((float)ev.x,(float)ev.y)) {
        int ms = maxScroll();
        if (ev.button > 0) m_scroll = std::max(0,  m_scroll - 2);
        else               m_scroll = std::min(ms, m_scroll + 2);
        return true;
    }

    if (ev.type == skEventType::MouseDown) {
        if (ur.contains((float)ev.x,(float)ev.y)) { navigateUp(); return true; }
        if (okR.contains((float)ev.x,(float)ev.y)) {
            if (!m_selectedPath.empty()) {
                setVisible(false); if (m_onConfirm) m_onConfirm(m_selectedPath);
            }
            return true;
        }
        if (canR.contains((float)ev.x,(float)ev.y)) {
            setVisible(false); if (m_onCancel) m_onCancel(); return true;
        }
        if (lr.contains((float)ev.x,(float)ev.y)) {
            int idx = entryAt(ev.y);
            if (idx >= 0) {
                if (idx == m_lastClickIdx) {
                    openEntry(idx);   // double-click: navigate/select
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
        // Click outside dialog
        float ddx = dlgX(), ddy = dlgY();
        bool inDlg = (ev.x>=(int)ddx && ev.x<(int)(ddx+kDlgW) &&
                      ev.y>=(int)ddy && ev.y<(int)(ddy+kDlgH));
        if (!inDlg) { setVisible(false); if (m_onCancel) m_onCancel(); }
        return true;
    }
    return true;
}
