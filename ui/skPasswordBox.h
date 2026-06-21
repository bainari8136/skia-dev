#pragma once
#include "skTextInput.h"

// Single-line password input — masks all characters with '*'.
// Inherits all skTextInput behaviour; only the display differs.
class skPasswordBox : public skTextInput {
public:
    skPasswordBox(int x, int y, int w, int h, std::string placeholder = "");
};
