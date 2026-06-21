#include "skTypeface.h"
#include <include/core/SkFontMgr.h>
#include <include/core/SkFontStyle.h>
#include <include/ports/SkTypeface_win.h>

sk_sp<SkTypeface> skGetSystemTypeface() {
    static sk_sp<SkTypeface> s_tf = []() -> sk_sp<SkTypeface> {
        auto mgr = SkFontMgr_New_DirectWrite();
        if (!mgr) return nullptr;
        return mgr->matchFamilyStyle("Segoe UI", SkFontStyle());
    }();
    return s_tf;
}
