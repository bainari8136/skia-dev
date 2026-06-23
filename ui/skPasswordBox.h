#pragma once
#include "skTextInput.h"
#include <memory>

// Single-line password input — masks all characters with '*'.
// Inherits all skTextInput behaviour; only the display differs.
class skPasswordBox : public skTextInput {
public:
    skPasswordBox(int x, int y, int w, int h, std::string placeholder = "");

    static std::shared_ptr<skPasswordBox> make(int x, int y, int w, int h,
                                                std::string placeholder = "");
};
