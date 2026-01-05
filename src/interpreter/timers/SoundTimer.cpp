#include <chrono>
#include "SoundTimer.h"

// Configuration to produce beep sound
static constexpr SDL_AudioFormat kFormat = AUDIO_S16SYS;
static constexpr uint8_t kChannels = 1;
static constexpr uint16_t kFrequency = 44100;
static constexpr uint16_t kBeepFrequency = 440;
static constexpr uint16_t kAmplitude = 8000;
static constexpr uint16_t kSampleCount = 1024;

// Function to produce our beep sound to pass to the output audio device
// https://wiki.libsdl.org/SDL2/SDL_AudioCallback
static void audioCallback(void* userdata, uint8_t* stream, int len) {
    auto* obtained = static_cast<SDL_AudioSpec*>(userdata);
    static int samplePos = 0;

    // Audio stream read by SDL
    int16_t* buffer = reinterpret_cast<int16_t*>(stream);
    int samples = len / sizeof(int16_t);

    int sampleRate = obtained->freq;
    int period = sampleRate / kBeepFrequency;

    for (int i = 0; i < samples; ++i) {
        ++samplePos;

        // Create square wave
        buffer[i] = (samplePos % period < period / 2) ? kAmplitude : -kAmplitude;
    }
}

// https://wiki.libsdl.org/SDL2/SDL_AudioSpec
SoundTimer::SoundTimer() {
    SDL_zero(m_desired);

    // Silence and size values are calculated by SDL
    m_desired.freq = kFrequency;
    m_desired.format = kFormat;
    m_desired.channels = kChannels;
    m_desired.samples = kSampleCount;
    m_desired.callback = audioCallback;
    m_desired.userdata = &m_obtained;

    // Use nullptr to use most reasonable default audio device
    // https://wiki.libsdl.org/SDL2/SDL_OpenAudioDevice
    m_audioDevice = SDL_OpenAudioDevice(
        nullptr, 0, &m_desired,
        &m_obtained, 0
    );

    if (!m_audioDevice)
        throw std::runtime_error(
            "Failed to initialise audio device: " + std::string(SDL_GetError())
        );

    m_timerThread = std::jthread(&SoundTimer::tickTimer, this);
}

SoundTimer::~SoundTimer() {
    // Make sure timer thread has completed before closing audio device
    m_timerThread.request_stop();
    m_timerThread.join();

    SDL_CloseAudioDevice(m_audioDevice);
}

void SoundTimer::tickTimer(std::stop_token stopToken) {
    while (!stopToken.stop_requested()) {
        // Decrement timer 60 times a second (60Hz)
        std::this_thread::sleep_for(
            std::chrono::duration<double>(1.0 / kTimerFrequency)
        );

        // Beep when timer is non-zero
        if (m_timer > 0) {
            // Beep! (unpause)
            SDL_PauseAudioDevice(m_audioDevice, kAudioPlay);
            m_isBeeping = true;

            // Decrement timer
            --m_timer;
        // Only pause if speaker is currently beeping
        } else if (m_isBeeping) {
            SDL_PauseAudioDevice(m_audioDevice, kAudioPause);
            m_isBeeping = false;
        }
    }
}

void SoundTimer::reset() {
    // End timer thread
    m_timerThread.request_stop();
    m_timerThread.join();

    // Reset state
    m_timer = 0;
    m_isBeeping = false;

    // Create new thread
    m_timerThread = std::jthread(&SoundTimer::tickTimer, this);
}

