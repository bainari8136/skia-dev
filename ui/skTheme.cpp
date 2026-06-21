#include "skTheme.h"

static skTheme s_theme;

skTheme& skGetTheme()             { return s_theme; }
void     skSetTheme(const skTheme& t) { s_theme = t; }

skTheme skTheme::light() { return skTheme{}; }

skTheme skTheme::dark() {
    skTheme t;
    t.windowBg       = SkColorSetRGB( 18,  19,  26);
    t.panelBg        = SkColorSetRGB( 28,  30,  42);
    t.panelBorder    = SkColorSetRGB( 52,  55,  75);
    t.accent         = SkColorSetRGB( 90, 150, 255);
    t.accentHover    = SkColorSetRGB(115, 170, 255);
    t.accentPress    = SkColorSetRGB( 60, 110, 220);
    t.textPrimary    = SkColorSetRGB(218, 220, 235);
    t.textSecondary  = SkColorSetRGB(140, 143, 165);
    t.textPlaceholder= SkColorSetRGB( 82,  85, 108);
    t.textOnAccent   = SK_ColorWHITE;
    t.inputBg        = SkColorSetRGB( 38,  40,  56);
    t.inputBorder    = SkColorSetRGB( 62,  65,  88);
    t.trackBg        = SkColorSetRGB( 52,  55,  75);
    return t;
}
