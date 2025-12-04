#pragma once

#include <SDL.h>
#include "Timer.h"

// Sound timer is write only
class SoundTimer : public Timer {
    static constexpr uint8_t kTimerFrequency = 60;
    static constexpr uint8_t kAudioPlay = 0;
    static constexpr uint8_t kAudioPause = 1;

    SDL_AudioSpec m_desired{}, m_obtained{};

    // Initialised in constructor
    // https://wiki.libsdl.org/SDL2/SDL_OpenAudioDevice
    SDL_AudioDeviceID m_audioDevice;

    bool isBeeping = false;

    void tickTimer(std::stop_token stopToken) override;

    public:
        SoundTimer();
        ~SoundTimer();
};
