#pragma once

#include "Timer.h"

// Delay timer is read+write
class DelayTimer : public Timer {
    public:
        uint8_t readTimer() {
            return m_timer;
        }

        void tickTimer() override {
            if (m_timer > 0)
                --m_timer;
        }

        void reset() override {
            // Reset count
            m_timer = 0;
        }
};
