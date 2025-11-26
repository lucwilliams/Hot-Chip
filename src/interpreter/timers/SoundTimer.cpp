#include "SoundTimer.h"

static constexpr uint16_t kFrequency = 8000;
static constexpr uint16_t kSampleRate = 256;

// Function to produce our beep sound to pass to the output audio device
// https://wiki.libsdl.org/SDL2/SDL_AudioCallback
[[maybe_unused]] static void audioCallback([[maybe_unused]] void* userdata, uint8_t* stream, int len) {
    static int samplePos = 0;
    constexpr uint16_t kAmplitude = 12000;

    // Audio stream read by SDL
    int16_t* buffer = reinterpret_cast<int16_t*>(stream);
    int samples = len / sizeof(int16_t);

    for (int i = 0; i < samples; ++i) {
        ++samplePos;
        int period = kSampleRate / kFrequency;

        // Create square wave
        int16_t sample = (samplePos % period < period / 2) ? kAmplitude : -kAmplitude;

        buffer[i] = sample;
    }
}

// https://wiki.libsdl.org/SDL2/SDL_AudioSpec
SoundTimer::SoundTimer() {
    // Silence and size values are calculated by SDL
    m_desired.freq = kFrequency;
    m_desired.format = kFormat;
    m_desired.channels = kChannels;
    m_desired.samples = kSampleRate;
    m_desired.callback = audioCallback;

    // Use nullptr to use most reasonable default audio device
    // https://wiki.libsdl.org/SDL2/SDL_OpenAudioDevice
    m_audioDevice = SDL_OpenAudioDevice(
        nullptr, 0, &m_desired,
        &m_obtained, 0
    );

    if (!m_audioDevice)
        throw std::runtime_error(
            "Failed to initialise audio device. " + std::string(SDL_GetError())
        );
}

void SoundTimer::tickTimer() {
    // Decrement timer 60 times a second (60Hz)
    std::this_thread::sleep_for(
        std::chrono::duration<double>(1.0/60.0)
    );
    --m_timer;

    // Beep when timer is non-zero
    if (m_timer > 0) {
        // Beep!
        SDL_PauseAudioDevice(m_audioDevice, kAudioPlay);
    } else {
        SDL_PauseAudioDevice(m_audioDevice, kAudioPause);
    }
}

