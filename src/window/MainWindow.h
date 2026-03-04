#pragma once

#include <format>
#include <SDL.h>
#include <nfd.hpp>
#include "../interpreter/Chip8DebugData.h"

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

    // The amount of instructions to be saved in the history toolbar.
    // Power of 2 is used for performant modulo operation.
    static constexpr std::uint16_t kInstructionHistorySize = 512;

    // SDL Window objects (nullptr initialised)
    SDL_Texture* m_texture{};
    SDL_Window* m_window{};
    SDL_Surface* m_surface{};
    SDL_Renderer* m_renderer{};

    // Full resolution pixel array, 1 byte -> 8 pixels
    std::span<std::uint8_t> m_frameBuffer{};

    // Contains the desired ROM path chosen by the UI.
    // Once m_desiredROMPath updates, Chip8 loads the ROM at that path.
    std::string m_desiredROMPath{};

    // Instruction history for debug UI
    RingBuffer<std::string, kInstructionHistorySize> m_instructionHistory{};

    // Only render pixels to the screen if the framebuffer has updated
    bool m_pixelsModified = false;

    public:
        MainWindow(std::string_view ROMPath);
        ~MainWindow();
        void render();
        void clearDisplay();
        void drawUI(Chip8DebugData debugInfo);
        void pushInstructionHistory(const std::string& instruction);
        bool drawRow(int x_index, int y_index, std::uint8_t rowData);
        std::string_view getROM();
        static NFD::UniquePath openFileBrowser();
};
