#pragma once

enum class skEventType {
    MouseDown,
    MouseUp,
    MouseMove,
    KeyChar,   // WM_CHAR  — printable + control chars (ch field)
    KeyDown    // WM_KEYDOWN — virtual key code (button field)
};

struct skEvent {
    skEventType type;
    int     x      = 0;
    int     y      = 0;
    int     button = 0;   // mouse button (0=left) OR virtual key code for KeyDown
    wchar_t ch     = 0;   // for KeyChar
};
