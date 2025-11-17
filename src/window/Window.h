#pragma once

#include <SDL.h>

class Window {
    // Window resolution constants
    static constexpr int kScreenWidth = 64;
    static constexpr int kScreenHeight = 32;
    static constexpr int kScreenUpscale = 25;
    static constexpr int kPixelCount = kScreenWidth * kScreenHeight;
    static constexpr int kScreenPitch = kScreenWidth / 8;

    // SDL Window objects (nullptr initialised)
    SDL_Texture* m_texture{};
    SDL_Window* m_window{};
    SDL_Surface* m_surface{};
    SDL_Renderer* m_renderer{};

    // Full resolution pixel array, 1 byte -> 8 pixels
    uint8_t* m_frameBuffer{};

    // Only render pixels to the screen if the framebuffer has updated
    bool m_pixelsModified = false;

    public:
        Window();
        ~Window();
        void render();
        void clearDisplay();
        bool drawRow(int x_index, int y_index, uint8_t rowData);
};
