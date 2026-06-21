#pragma once

enum class skEventType {
    MouseDown,
    MouseUp,
    MouseMove
};

struct skEvent {
    skEventType type;
    int x, y;
    int button; // 0=left, 1=right, 2=middle
};
