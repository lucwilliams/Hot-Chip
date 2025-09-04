#pragma once

#include <SDL.h>

namespace WindowConfig {
    constexpr int kScreenWidth = 64;
    constexpr int kScreenHeight = 32;
    constexpr int kScreenUpscale = 25;
    constexpr int kScreenResolution = kScreenWidth * kScreenHeight;
    constexpr int kScreenPitch = kScreenWidth / 8;
}

class Window {
    // SDL Window objects
    SDL_Texture* m_texture {};
    SDL_Window* m_window {};
    SDL_Surface* m_surface {};
    SDL_Renderer* m_renderer {};

    // Full resolution pixel array, 1 byte -> 8 pixels
    uint8_t* m_frameBuffer {};

    // Only draw pixels to screen if the framebuffer has updated
    bool m_pixelsModified {false};

    public:
        Window();
        ~Window();
        void Draw();
        void flipPixel(const int& x_index, const int& y_index);
        void Debug_Print() const;
};
