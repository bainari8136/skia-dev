#pragma once
#include "skWidget.h"

// Indeterminate loading spinner.  Draws a 270-degree arc rotating at
// ~1 revolution per second when running.  Renders nothing when stopped.
class skSpinner : public skWidget {
public:
    skSpinner(int x, int y, int size);

    void start()                { m_running = true;  m_autoStopTicks = -1; }
    void stop()                 { m_running = false; }
    void runFor(int ticks)      { m_running = true;  m_autoStopTicks = ticks; }
    bool running()        const { return m_running; }

    void Paint(SkCanvas* canvas) override;
    void onTick()                override;

private:
    bool  m_running        = false;
    float m_angle          = 0.f;
    int   m_autoStopTicks  = -1; // -1 = run until stop() is called
};
