#pragma once

#include "Timer.h"

// Delay timer is read+write
class DelayTimer : public Timer {
    public:
        void tickTimer();
        uint8_t readTimer();
};
