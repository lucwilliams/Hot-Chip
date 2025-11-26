#include "DelayTimer.h"

void DelayTimer::tickTimer() {
    // Decrement timer 60 times a second (60Hz)
    std::this_thread::sleep_for(
        std::chrono::duration<double>(1.0/60.0)
    );
    --m_timer;
}

uint8_t DelayTimer::readTimer() {
    return m_timer;
}
