#pragma once

#include <SDL.h>
#include <nfd.hpp>
#include "../interpreter/Chip8MemoryView.h"

class MainWindow {
    // Window resolution constants
    static constexpr int kScreenWidth = 64;
    static constexpr int kScreenHeight = 32;
    static constexpr int kScreenViewPortUpscale = 15;
    static constexpr int kScreenUpscale = 25;
    static constexpr int kPixelCount = kScreenWidth * kScreenHeight;

    // Divide pixel count by 8 for byte amount. (8 pixels per byte)
    static constexpr int kPackedPixelCount = kPixelCount / 8;
    static constexpr int kScreenPitch = kScreenWidth / 8;

    // SDL Window objects (nullptr initialised)
    SDL_Texture* m_texture{};
    SDL_Window* m_window{};
    SDL_Surface* m_surface{};
    SDL_Renderer* m_renderer{};

    // Full resolution pixel array, 1 byte -> 8 pixels
    std::span<uint8_t> m_frameBuffer{};

    // Only render pixels to the screen if the framebuffer has updated
    bool m_pixelsModified = false;

    public:
        MainWindow();
        ~MainWindow();
        void render();
        void clearDisplay();
        void drawUI(Chip8MemoryView debugInfo);
        bool drawRow(int x_index, int y_index, uint8_t rowData);
        static NFD::UniquePath openFileBrowser();
};
