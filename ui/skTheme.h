#pragma once
#include <include/core/SkColor.h>

struct skTheme {
    // Surfaces
    SkColor windowBg      = SkColorSetRGB(240, 242, 247);
    SkColor panelBg       = SK_ColorWHITE;
    SkColor panelBorder   = SkColorSetRGB(218, 220, 228);

    // Accent
    SkColor accent        = SkColorSetRGB( 55, 120, 220);
    SkColor accentHover   = SkColorSetRGB( 80, 150, 255);
    SkColor accentPress   = SkColorSetRGB( 20,  80, 180);

    // Text
    SkColor textPrimary   = SkColorSetRGB( 20,  20,  32);
    SkColor textSecondary = SkColorSetRGB( 80,  80,  95);
    SkColor textPlaceholder = SkColorSetRGB(165, 165, 175);
    SkColor textOnAccent  = SK_ColorWHITE;

    // Inputs / tracks
    SkColor inputBg       = SK_ColorWHITE;
    SkColor inputBorder   = SkColorSetRGB(190, 192, 200);
    SkColor trackBg       = SkColorSetRGB(210, 212, 220);

    static skTheme light();
    static skTheme dark();
};

skTheme& skGetTheme();
void     skSetTheme(const skTheme& t);
