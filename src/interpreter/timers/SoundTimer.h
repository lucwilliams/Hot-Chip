#pragma once

#include "Timer.h"

// Sound timer is write only
class SoundTimer : public Timer {
    public:
        // Initialise as a timer that can beep
        SoundTimer() : Timer(true) {}
};
