#include "skPasswordBox.h"

skPasswordBox::skPasswordBox(int px, int py, int pw, int ph, std::string placeholder)
    : skTextInput(px, py, pw, ph, std::move(placeholder)) {
    setMasked(true);
}
