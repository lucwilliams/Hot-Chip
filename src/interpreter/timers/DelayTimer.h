#pragma once

#include "Timer.h"

// Delay timer is read+write
class DelayTimer : public Timer {
    void tickTimer(std::stop_token stopToken) override;

    public:
        DelayTimer();
        uint8_t readTimer();
        void reset() override;
        ~DelayTimer() override;
};
