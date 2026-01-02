#include <iostream>
#include <cstring>
#include <imgui.h>
#include <imgui_impl_sdl2.h>
#include <imgui_impl_sdlrenderer2.h>
#include <imgui_memory_editor.h>
#include "Window.h"

// Initialise program window
Window::Window() {
    // Initialise SDL
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) < 0) {
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
        m_window, -1, SDL_RENDERER_ACCELERATED
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

    // Initialise ImGUI for debugging
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui::StyleColorsDark();

    // Enable docking in ImGUI
    ImGui::GetIO().ConfigFlags |= ImGuiConfigFlags_DockingEnable;

    // Use SDL Renderer for ImGUI
    ImGui_ImplSDL2_InitForSDLRenderer(m_window, m_renderer);
    ImGui_ImplSDLRenderer2_Init(m_renderer);

    // Initialise texture surface for 2D rendering
    // Use SDL_PIXELFORMAT_INDEX1LSB for 1-bit pixels
    m_surface = SDL_CreateRGBSurfaceWithFormat(
        0, kScreenWidth, kScreenHeight,
        SDL_BITSPERPIXEL(SDL_PIXELFORMAT_INDEX1MSB),
        SDL_PIXELFORMAT_INDEX1MSB
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

void Window::render() {
    // Only update display texture when the framebuffer has been modified
    if (m_pixelsModified) {
        SDL_DestroyTexture(m_texture);

        // Create new texture. m_surface->pixels is our framebuffer
        m_texture = SDL_CreateTextureFromSurface(m_renderer, m_surface);

        // Pixels in framebuffer queue have been drawn
        m_pixelsModified = false;
    }
}

void Window::clearDisplay() {
    // Zero out framebuffer to completely clear it
    // Divide pixel count by 8 for byte amount. (8 pixels per byte)
    std::memset(m_frameBuffer, 0, kPixelCount / 8);

    // Update frame
    m_pixelsModified = true;
}

/*
 * drawUI() renders all ImGUI windows, including debug windows and
 * the emulated viewport. The render() function is responsible
 * for updating the emulated display to be used for this function.
 *
 * Chip8MemoryView only contains references so it can be copied as an argument.
 */
void Window::drawUI(Chip8MemoryView debugInfo) {
    // Update ImGUI UI
    ImGui_ImplSDLRenderer2_NewFrame();
    ImGui_ImplSDL2_NewFrame();
    ImGui::NewFrame();

    // Create full window dockspace
    ImGui::DockSpaceOverViewport(
        0, ImGui::GetMainViewport(), ImGuiDockNodeFlags_PassthruCentralNode
    );

    // Set default size of emulated display
    ImGui::SetNextWindowSize(
        // Scaled width and height
        {
            kScreenWidth * kScreenViewPortUpscale,
            kScreenHeight * kScreenViewPortUpscale
        },
        ImGuiCond_FirstUseEver
    );

    // Create window for emulated display
    ImGui::Begin(
        "Chip-8",
        nullptr,
        ImGuiWindowFlags_NoScrollbar |
        ImGuiWindowFlags_NoScrollWithMouse
    );

    ImVec2 size = ImGui::GetContentRegionAvail();

    ImGui::Image(
        m_texture,
        size,
        ImVec2(0, 0),
        ImVec2(1, 1)
    );

    ImGui::End();

    // Set default size of memory viewer
    ImGui::SetNextWindowSize({564, 439}, ImGuiCond_FirstUseEver);

    // Create memory viewer window
    ImGui::Begin("Chip-8 Memory");
    static MemoryEditor memoryViewer;

    std::span<uint8_t>& memory = debugInfo.memory;
    memoryViewer.DrawContents(
        memory.data(), memory.size()
    );

    ImGui::End();

    // Update ImGUI
    ImGui::Render();

    // Ensure UI fits in window
    ImGuiIO& io = ImGui::GetIO();
    SDL_RenderSetScale(
        m_renderer, io.DisplayFramebufferScale.x, io.DisplayFramebufferScale.y
    );

    // Clean up last render
    SDL_RenderClear(m_renderer);

    // Update ImGUI
    ImGui_ImplSDLRenderer2_RenderDrawData(ImGui::GetDrawData(), m_renderer);

    // Present
    SDL_RenderPresent(m_renderer);
}

// Start a position x, XOR x and the 7 following bits with rowData.
// If the next 7 bits are in the following byte, continue flipping bits
// in the second byte until the sprite byte is drawn.
bool Window::drawRow(int x_index, int y_index, uint8_t rowData) {
    // If position values exceed screen limits, wrap around.
    x_index %= kScreenWidth;
    y_index %= kScreenHeight;

    // Return true if any set bit becomes unset
    bool bitUnset = false;

    // Framebuffer has been modified, pixels await being drawn
    m_pixelsModified = true;

    // Y position is multiplied by the pitch (bytes per row)
    // Then we add the floor division of index / 8 to find the pixel's
    // corresponding byte (8 pixels per byte)
    uint8_t pos = (kScreenPitch * y_index) + x_index / 8;

    // Determine bit index within byte
    uint8_t bitPos = x_index % 8;

    // Create XOR mask for first byte (unused bits are unset)
    uint8_t firstXOR = rowData >> bitPos;
    uint8_t& row = m_frameBuffer[pos];

    // AND the current row with the mask.
    // If two bits match, a set bit is flipped,T
    // making the AND operation nonzero.
    if ((row & firstXOR) != 0) {
        bitUnset = true;
    }

    // XOR the first byte to flip pixels
    row ^= firstXOR;

    // The starting bit wasn't at the beginning of a byte,
    // we must continue flipping bits in the next byte
    // to complete the drawing of the sprite byte.
    if (bitPos != 0) {
        // Set up new mask
        uint8_t secondXOR = rowData << (8 - bitPos);

        // Move to the next byte
        ++pos;
        uint8_t& nextRow = m_frameBuffer[pos];

        if ((nextRow & secondXOR) != 0) {
            bitUnset = true;
        }

        // XOR the second byte to flip pixels
        nextRow ^= secondXOR;
    }

    return bitUnset;
}

Window::~Window() {
    // Close ImGUI
    ImGui_ImplSDLRenderer2_Shutdown();
    ImGui_ImplSDL2_Shutdown();
    ImGui::DestroyContext();

    // Close SDL
    SDL_DestroyTexture(m_texture);
    SDL_DestroyRenderer(m_renderer);
    SDL_DestroyWindow(m_window);
    SDL_Quit();
}
