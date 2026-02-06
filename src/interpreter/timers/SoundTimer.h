#pragma once

#include <SDL.h>
#include "Timer.h"

// Sound timer is write only
class SoundTimer : public Timer {
    static constexpr std::uint8_t kTimerFrequency = 60;
    static constexpr std::uint8_t kAudioPlay = 0;
    static constexpr std::uint8_t kAudioPause = 1;

    SDL_AudioSpec m_desired{}, m_obtained{};

    // Initialised in constructor
    // https://wiki.libsdl.org/SDL2/SDL_OpenAudioDevice
    SDL_AudioDeviceID m_audioDevice;

    bool m_isBeeping = false;

    public:
        SoundTimer();
        ~SoundTimer() override;
        void tickTimer() override;
        void reset() override;
};
