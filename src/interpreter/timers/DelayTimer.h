#pragma once

#include "Timer.h"

// Delay timer is read+write
class DelayTimer : public Timer {
    public:
        inline uint8_t readTimer() {
            return m_timer;
        }
};
