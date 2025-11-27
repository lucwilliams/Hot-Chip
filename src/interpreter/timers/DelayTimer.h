#pragma once

#include "Timer.h"

// Delay timer is read+write
class DelayTimer : public Timer {
    public:
        DelayTimer();
        ~DelayTimer();
        void tickTimer(std::stop_token stopToken) override;
        uint8_t readTimer();
};
