#include <iostream>
#include <cstring>
#include <imgui.h>
#include <imgui_impl_sdl2.h>
#include <imgui_impl_sdlrenderer2.h>
#include <imgui_memory_editor.h>
#include <imgui_internal.h>
#include <nfd_sdl2.h>
#include "MainWindow.h"

// ImGUI flags to make windows unmovable
constexpr int kLockedWindowFlags =
    ImGuiWindowFlags_NoMove |
    ImGuiWindowFlags_NoResize |
    ImGuiWindowFlags_NoCollapse
;

// Initialise program window
MainWindow::MainWindow() {
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

    // Initialise ImGUI for debug UI
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui::StyleColorsDark();

    // Enable docking in ImGUI
    ImGui::GetIO().ConfigFlags |= ImGuiConfigFlags_DockingEnable;

    // Disable ImGUI .ini settings file
    ImGui::GetIO().IniFilename = nullptr;

    // Use SDL Renderer for ImGUI
    ImGui_ImplSDL2_InitForSDLRenderer(m_window, m_renderer);
    ImGui_ImplSDLRenderer2_Init(m_renderer);

    // Initialise NFDe for file browser UI
    NFD::Init();

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

    /*
     * Framebuffer of the resolution of the 64x32 display (256 bytes)
     * Each byte represents 8 pixels. Framebuffer represents the
     * original resolution, not upscaled.
     */
    m_frameBuffer = {static_cast<uint8_t*>(m_surface->pixels), kPackedPixelCount};
}

void MainWindow::render() {
    // Only update display texture when the framebuffer has been modified
    if (m_pixelsModified) {
        SDL_DestroyTexture(m_texture);

        // Create new texture. m_surface->pixels is our framebuffer
        m_texture = SDL_CreateTextureFromSurface(m_renderer, m_surface);

        // Pixels in framebuffer queue have been drawn
        m_pixelsModified = false;
    }
}

void MainWindow::clearDisplay() {
    // Zero out framebuffer to completely clear it
    std::ranges::fill(m_frameBuffer, 0);

    // Update frame
    m_pixelsModified = true;
}


NFD::UniquePath MainWindow::openFileBrowser() {
    NFD::UniquePath outPath;
    nfdfilteritem_t fileTypeFilter[1] = {
        {"Chip-8 ROM", "ch8,bin"}
    };

    nfdresult_t result = NFD::OpenDialog(outPath, fileTypeFilter, 1);

    // Set output to nullptr if the user failed to choose a valid file
    if (result != NFD_OKAY) {
        outPath = nullptr;
    }

    return outPath;
}

/*
 * drawUI() renders all ImGUI windows, including debug windows and
 * the emulated viewport. The render() function is responsible
 * for updating the emulated display to be used for this function.
 *
 * Chip8MemoryView only contains references so it can be copied as an argument.
 */
void MainWindow::drawUI(Chip8MemoryView debugInfo) {
    // Update ImGUI UI
    ImGui_ImplSDLRenderer2_NewFrame();
    ImGui_ImplSDL2_NewFrame();
    ImGui::NewFrame();

    ImGuiID fullDockspaceID = ImGui::GetID("Full Dockspace");

    // Use dockBuilder to form window
    if (ImGui::DockBuilderGetNode(fullDockspaceID) == nullptr)
    {
        // Register full dockspace
        ImGui::DockBuilderAddNode(
            fullDockspaceID, ImGuiDockNodeFlags_DockSpace
        );

        // Use full viewport for dockspace
        ImGui::DockBuilderSetNodeSize(
            fullDockspaceID, ImGui::GetMainViewport()->Size
        );

        // Initialise split dock IDs
        ImGuiID displayDockspaceID = fullDockspaceID;

        ImGuiID leftSideDockID{0};
        ImGuiID rightSideDockID{0};
        ImGuiID memoryViewerDockID{0};
        ImGuiID controlsDockID{0};

        /*
         * Split up the dockspace to create a left vertical dock,
         * using 20% of the width of the screen
         */
        ImGui::DockBuilderSplitNode(
            displayDockspaceID, ImGuiDir_Left, 0.20f,
            &leftSideDockID, &displayDockspaceID
        );

        // Repeat for the right vertical dock
        ImGui::DockBuilderSplitNode(
            displayDockspaceID, ImGuiDir_Right, 0.20f,
            &rightSideDockID, &displayDockspaceID
        );

        // Split the main window down the middle to place debug UI below display
        ImGui::DockBuilderSplitNode(
            displayDockspaceID, ImGuiDir_Down, 0.40f,
            &memoryViewerDockID, &displayDockspaceID
        );

        // Split the bottom middle window to place memory viewer next to control buttons
        ImGui::DockBuilderSplitNode(
            memoryViewerDockID, ImGuiDir_Right, 0.30f,
            &controlsDockID, &memoryViewerDockID
        );

        ImGui::DockBuilderDockWindow("Chip-8", displayDockspaceID);
        ImGui::DockBuilderDockWindow("Chip-8 Memory", memoryViewerDockID);
        ImGui::DockBuilderDockWindow("Chip-8 Controls", controlsDockID);
        ImGui::DockBuilderDockWindow("Registers", leftSideDockID);
        ImGui::DockBuilderDockWindow("Instructions", rightSideDockID);

        ImGui::DockBuilderFinish(fullDockspaceID);
    }

    // Create full window dockspace
    ImGui::DockSpaceOverViewport(
        fullDockspaceID, ImGui::GetMainViewport(), ImGuiDockNodeFlags_PassthruCentralNode
    );

    // Create window for emulated display
    ImGui::Begin(
        "Chip-8",
        nullptr,
        // Lock window and prevent scroll
        kLockedWindowFlags |
        ImGuiWindowFlags_NoScrollbar |
        ImGuiWindowFlags_NoScrollWithMouse
    );

    ImVec2 size = ImGui::GetContentRegionAvail();

    // Use SDL_Texture (frame data of the emulated display) as an ImGUI image
    ImGui::Image(
        m_texture,
        size,
        ImVec2(0, 0),
        ImVec2(1, 1)
    );

    ImGui::End();

    // Create memory viewer window
    ImGui::Begin(
        "Chip-8 Memory",
        nullptr,
        kLockedWindowFlags
    );
    static MemoryEditor memoryViewer;

    std::span<uint8_t> memory = debugInfo.memory;
    memoryViewer.DrawContents(
        memory.data(), memory.size()
    );

    ImGui::End();

    // Create control window for emulator state
    ImGui::Begin(
        "Chip-8 Controls",
        nullptr,
        kLockedWindowFlags
    );

    if (ImGui::Button("Load ROM")) {
        NFD::UniquePath ROMPath = openFileBrowser();

        // If the user provided a valid file path
        if (ROMPath) {
            // Cast C string to std::array from underlying std::unique_ptr
            *debugInfo.sharedROMPath = std::string(ROMPath.get());
        }
    }

    ImGui::End();

    ImGui::Begin(
        "Registers",
        nullptr,
        kLockedWindowFlags
    );

    // Create table for register values
    if (ImGui::BeginTable("Registers", 1, 0))
    {
        ImGui::TableSetupColumn("Registers");
        ImGui::TableHeadersRow();

        constexpr uint8_t kRegisterAmount = 16;
        for (int row = 0; row < kRegisterAmount; row++)
        {
            ImGui::TableNextRow();
            ImGui::TableSetColumnIndex(0);
            ImGui::Text("V%d: %d", row, debugInfo.registers[row]);
        }

        // Add special registers
        ImGui::Text("Index: %d", debugInfo.index);
        ImGui::Text("PC: %d", debugInfo.PC);

        ImGui::EndTable();
    }

    ImGui::End();

    ImGui::Begin(
        "Instructions",
        nullptr,
        kLockedWindowFlags
    );

    // Create table for instructions (WIP)
    if (ImGui::BeginTable("Instructions", 1, 0))
    {
        ImGui::TableSetupColumn("Instructions");
        ImGui::TableHeadersRow();

        ImGui::Text("mov WIP, WIP");

        ImGui::EndTable();
    }

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
bool MainWindow::drawRow(int x_index, int y_index, uint8_t rowData) {
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

MainWindow::~MainWindow() {
    // Close NFDe
    NFD::Quit();

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
