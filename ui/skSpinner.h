#pragma once
#include "skWidget.h"

// Indeterminate loading spinner.  Draws a 270-degree arc rotating at
// ~1 revolution per second when running.  Renders nothing when stopped.
class skSpinner : public skWidget {
public:
    skSpinner(int x, int y, int size);

    void start()         { m_running = true; }
    void stop()          { m_running = false; }
    bool running() const { return m_running; }

    void Paint(SkCanvas* canvas) override;
    void onTick()        override;

private:
    bool  m_running  = false;
    float m_angle    = 0.f; // degrees, head of the sweep arc
};
