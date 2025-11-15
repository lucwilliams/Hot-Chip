#include <cstdint>
#include <iostream>
#include <cstring>
#include "Window.h"

// Initialise program window
Window::Window() {
    // Initialise SDL
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        std::cerr << "Error initialising SDL." << std::endl;
    }

    // Create a window (640x320 pixels)
    m_window =
        SDL_CreateWindow(
            "Hot-Chip",                     // Window title
            SDL_WINDOWPOS_CENTERED,         // X position
            SDL_WINDOWPOS_CENTERED,         // Y position
            kScreenWidth * kScreenUpscale,  // Width (x upscale)
            kScreenHeight * kScreenUpscale, // Height (x upscale)
            SDL_WINDOW_SHOWN                // Flags
        );

    if (!m_window) {
        std::cerr << "Error initialising window." << std::endl;
    }

    m_renderer = SDL_CreateRenderer(
        m_window, -1, SDL_RENDERER_PRESENTVSYNC
    );

    if (!m_renderer) {
        std::cerr << "Error initialising renderer." << std::endl;
    }

    // Set logical size of window to be 64x32
    if (
        SDL_RenderSetLogicalSize(m_renderer, kScreenWidth, kScreenHeight) < 0
    ) {
        std::cerr << "Error sizing renderer." << std::endl;
    }

    // Use nearest-neighbour scaling
    SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "nearest");

    // Use SDL_PIXELFORMAT_INDEX1LSB for 1-bit pixels
    m_surface = SDL_CreateRGBSurfaceWithFormat(
        0, kScreenWidth, kScreenHeight, 1,
       SDL_PIXELFORMAT_INDEX1LSB
    );

    if (!m_surface) {
        std::cerr << "Error creating surface." << std::endl;
    }

    m_texture = SDL_CreateTextureFromSurface(m_renderer, m_surface);

    if (!m_texture) {
        std::cerr << "Error creating texture." << std::endl;
    }

    // Create colour palette of either black or white (off/on bits)
    constexpr SDL_Color monochrome[2] = {
        {0, 0, 0, 255},
        {255, 255, 255, 255}
    };

    SDL_SetPaletteColors(m_surface->format->palette, monochrome, 0, 2);

    // Framebuffer of the resolution of the 64x32 display (256 bytes)
    // Each byte represents 8 pixels. Framebuffer represents original
    // resolution, not upscaled
    m_frameBuffer = static_cast<uint8_t*>(m_surface->pixels);
}

void Window::draw() {
    // Only draw when the framebuffer has been modified
    if (m_pixelsModified) {
        // Clean up last draw
        SDL_RenderClear(m_renderer);
        SDL_DestroyTexture(m_texture);

        // Create new texture. m_surface->pixels is our framebuffer
        m_texture = SDL_CreateTextureFromSurface(m_renderer, m_surface);

        // Copy and draw
        SDL_RenderCopy(m_renderer, m_texture, nullptr, nullptr);
        SDL_RenderPresent(m_renderer);

        // Pixels in framebuffer queue have been drawn
        m_pixelsModified = false;
    }
}

void Window::flipPixel(int x_index, int y_index) {
    // Framebuffer has been modified, pixels await being drawn
    m_pixelsModified = true;

    // Y position is multiplied by the pitch (bytes per row)
    // Then we add the floor division of index / 8 to find the pixel's
    // corresponding byte (8 pixels per byte)
    const uint8_t pos = (kScreenPitch * y_index) + x_index / 8;

    // Pixel's corresponding byte to be modified
    uint8_t& pixelsByte = m_frameBuffer[pos];

    // Determine bit index within byte
    const uint8_t bitPos = x_index % 8;

    // Set a single bit in the position of the pixel
    const uint8_t bitIndex = 1 << bitPos;

    // XOR the byte by our single set bit to swap the pixel
    pixelsByte ^= bitIndex;
}

void Window::clearDisplay() {
    // Zero out framebuffer to completely clear it
    std::memset(m_frameBuffer, 0, 256);

    // Update frame
    m_pixelsModified = true;
    draw();
}

Window::~Window() {
    SDL_DestroyTexture(m_texture);
    SDL_DestroyWindow(m_window);
    SDL_DestroyRenderer(m_renderer);
    SDL_Quit();
}
