#include <fstream>
#include <cerrno>
#include <cstring>
#include <iostream>
#include <algorithm>
#include <imgui.h>
#include <imgui_impl_sdl2.h>
#include <chrono>
#include "Chip8.h"

Chip8::Chip8(const std::string& fileName, MainWindow& window)
	: m_ROMPath{fileName}
	, m_window{window}
{
    #if defined(_WIN64)
    	// Initialise high resolution timer on Windows
		m_winTimerHandle = CreateWaitableTimerExW(nullptr, nullptr,
			CREATE_WAITABLE_TIMER_HIGH_RESOLUTION,
			TIMER_ALL_ACCESS
		);
    #endif

	loadROM();
}

void Chip8::loadROM() {
	// Read ROM data
	std::ifstream inFS;

	// Open the file to read in binary
	// Use ifstream::ate to position the get pointer at the end of file
	inFS.open(m_ROMPath, std::ifstream::binary | std::ifstream::ate);

	if (inFS.is_open()) {
		// Determine ROM size by checking the position of the get pointer
		m_ROMSize = static_cast<uint16_t>(inFS.tellg());

		// Check that ROM isn't greater than the space allocated to it in program memory
		// 4096 - 512 (offset of ROM in memory)
		if (m_ROMSize > (kMemorySize - kROMOffset))
			throw std::runtime_error("ROM size exceeds maximum of 3584 bytes: " + m_ROMPath);

		// Return get pointer to start of file
		inFS.seekg(0, std::ifstream::beg);

		// Read in ROM to emulated memory, offsetting by 512 bytes.
		// The initial 512 bytes of the Chip-8 memory was used to store
		// the interpreter code in the original hardware.
		inFS.read(reinterpret_cast<char*>(m_memory.begin() + kROMOffset), m_ROMSize);

		// Initialise font data. Start font data in position 0x50 (+80 bytes) as is conventional.
		std::copy(kFontData.begin(), kFontData.end(), m_memory.begin() + kFontOffset);

		// Update m_sharedROMPath with currently loaded ROM
		*m_sharedROMPath = m_ROMPath;
	} else {
		throw std::runtime_error("Error opening ROM: " + m_ROMPath + ", " + std::strerror(errno));
	}
}

void Chip8::resetEmulator() {
	// Reset memory and registers
	m_memory.clear();
	m_registers.clear();
	m_index = 0;
	m_PC = kROMOffset;

	// Clear stack
	m_stack.fill(0);
	m_stackSize = 0;

	// Reset timers
	m_soundTimer.reset();
	m_delayTimer.reset();

	// Clear framebuffer
	m_window.clearDisplay();
}

void Chip8::decode(uint16_t instruction) {
	// Fourth nibble is the most significant
	const uint8_t highestNibble = nibbleAt(instruction, 3);

    // Keep track of whether PC has updated
    m_PCUpdated = false;

	// Map instruction to opcode function call
	switch (highestNibble) {
		case 0x0:
			// CLEAR_DISPLAY and RETURN instructions
			opcode0(instruction);
			break;
        case 0x1:
            // Jump to NNN
            opcode1(instruction);
            break;
        case 0x2:
            // Call subroutine at NNN
            opcode2(instruction);
            break;
        case 0x3:
            // Skip next instruction if (Vx == NN)
            opcode3(instruction);
            break;
		case 0x4:
			// Skip next instruction if (Vx != NN)
			opcode4(instruction);
			break;
		case 0x5:
			// Skip next instruction if (Vx == Vy)
			opcode5(instruction);
			break;
		case 0x6:
			// Set register VX to NN
			opcode6(instruction);
			break;
		case 0x7:
			// Add NN to VX
			opcode7(instruction);
			break;
        case 0x8:
            // Math and bitwise operations on registers
            opcode8(instruction);
            break;
		case 0x9:
			// Skip next instruction if (Vx != Vy)
			opcode9(instruction);
			break;
		case 0xA:
			// Set I to NNN
			opcodeA(instruction);
			break;
		case 0xB:
			// Jump to NNN plus the value in V0
			opcodeB(instruction);
			break;
		case 0xC:
			// Generate a random number in range of 0 -> NN (max 255)
			opcodeC(instruction);
			break;
		case 0xD:
			// Draw sprite at (VX, VY) with height N
			opcodeD(instruction);
			break;
        case 0xE:
            // Skip instruction for key press
            opcodeE(instruction);
            break;
        case 0xF:
            // Timers, keystrokes and misc memory instructions involving VX
            opcodeF(instruction);
            break;
		default:
			if (kDebugEnabled)
				std::cout << "[DEBUG] Unknown opcode: " << instruction << std::endl;
	}
}

void Chip8::start() {
	#if defined(_WIN64)
		// Request Windows to allow this program to use higher precision sleep timing.
		timeBeginPeriod(1);
	#endif

	// Run emulator until window closes
	while (!m_windowClosed) {
		executionLoop();

		/*
		 * If the last emulation ended but the window didn't close,
		 * we know the emulation ended because the user
		 * requested a new ROM to be loaded.
		*/
		if (!m_windowClosed) {
			// Update ROM file path with new path requested
			m_ROMPath = *m_sharedROMPath;

			// Reset emulator state and load ROM
			resetEmulator();
			loadROM();
		}
	}
}

void Chip8::executionLoop() {
	// The memory location of the ROM's final valid instruction
	// Subtract two since instructions are two bytes in size.
	const uint16_t finalInstruction {
		static_cast<uint16_t>(kROMOffset + m_ROMSize - 2)
	};

	// Whether all instructions have been executed
	bool finished = false;

	// Fetch/decode/execute loop
	while (!m_windowClosed) {
		// * frame begins here *
		const auto frameStart = std::chrono::steady_clock::now();

		// Check if user has loaded a new ROM
		if (m_ROMPath != *m_sharedROMPath) {
			return;
		}

		if (finished) {
			// Sleep until the user closes the window
			SDL_WaitEvent(&m_event);

			if (m_event.type == SDL_QUIT) {
				m_windowClosed = true;
				return;
			}
		} else {
			// Get user inputs to update keyboard state at frame start
			while (SDL_PollEvent(&m_event)) {
				// Pass event to ImGUI
				ImGui_ImplSDL2_ProcessEvent(&m_event);

				// Scancode of key (zero if event is not a keypress)
				SDL_Scancode& scanCode = m_event.key.keysym.scancode;

				// Terminate execution on window close event
				if (m_event.type == SDL_QUIT) {
					m_windowClosed = true;
					return;
				}

				// Update key state to pressed
				if (m_event.type == SDL_KEYDOWN) {
					setKeyState(scanCode, true);

					/*
					 * If AWAIT_KEY was called and a key hasn't been pressed previously,
					 * save the scancode of this pressed key to await release.
					 */
					if (m_awaitingKey && m_awaitingScanCode == kNULLScanCode)
						m_awaitingScanCode = scanCode;

				// Update key state to not pressed
				} else if (m_event.type == SDL_KEYUP) {
					setKeyState(scanCode, false);

					/*
					 * If AWAIT_KEY was called and a key has been pressed,
					 * update VX to the released key's value.
					 */
					if (m_awaitingKey && m_awaitingScanCode != kNULLScanCode) {
						uint8_t& VX = m_registers[m_awaitingKeyRegNum];
						VX = scanCodeToPos(scanCode);

						// Restore variables for next AWAIT_KEY call
						m_awaitingKey = false;
						m_awaitingScanCode = kNULLScanCode;
					}
				}
			}
		}

		// Count the number of instructions executed per frame for timing emulation (IPF)
		uint16_t instructionsExecuted{0};

		// Execute a certain number of instructions per frame (~12).
		// Don't continue execution if we're currently awaiting a key press in AWAIT_KEY instruction.
		while (instructionsExecuted < kInstructionsPerFrame && !m_awaitingKey) {
            if (m_PC > finalInstruction) {
                // All instructions have completed
                finished = true;
            } else if (m_PC + 1 < kMemorySize) {
                /*
                 * Fetch instruction:
                 * Our memory is 8 bits, but an instruction is 16 bits.
                 * We concatenate the byte at PC with the byte that follows to form one uint16_t
                 */
                uint16_t instruction = static_cast<uint16_t>(m_memory[m_PC]) << 8 | m_memory[m_PC + 1];
                decode(instruction);

            	// Increment instruction count
            	instructionsExecuted++;

                // Do not increment PC for jump or return instructions
                if (!m_PCUpdated)
                    // Increment PC by 2 as instructions are two bytes in size
                    m_PC += 2;
            } else {
                throw std::runtime_error(
                    "Cannot run out-of-bounds instruction at position: "
                    + std::to_string(m_PC) + "."
                );
            }
		}

		// Create struct to pass read-only debug info to Window
		Chip8MemoryView debugInfo {
			m_memory.getDataView(),
			m_registers.getDataView(),
			m_PC,
			m_index,
			m_sharedROMPath
		};

		// Render frame (no change if no draw/clear calls made)
		m_window.render();

		// Render ImGUI UI
		m_window.drawUI(debugInfo);

		/*
		 * CHIP-8's timers decrement at the same pace as the framerate.
		 * Therefore, we tick each timer once per frame.
		 */
		m_delayTimer.tickTimer();
		m_soundTimer.tickTimer();

		/*
		 * Busy waiting logic is derived from Dolphin Emulator:
		 * (permissible under GPL-2.0-or-later)
		 * https://github.com/dolphin-emu/dolphin/blob/master/Source/Core/Common/Timer.cpp
		 *
		 * Windows implementation is informed by Blat Blatnik's research on high-accuracy sleep:
		 * https://blog.bearcats.nl/perfect-sleep-function/
		 */

		 /*
		 * 1 millisecond of spin time seems like a good balance between
		 * accuracy and performance on both Windows and Linux.
		 */
		constexpr auto spinDuration = std::chrono::milliseconds{1};

		// The expected end time of the frame
		const auto frameEnd = frameStart + kFrameDuration;

		// Actual end time of the frame
		const auto frameComplete = std::chrono::steady_clock::now();

		// If the frame completed with time to spare, sleep until the next frame
		if (frameComplete < frameEnd) {
			#if defined(_WIN64)
				// SetWaitableTimerEx takes time in "100 nanosecond intervals". (Credit: Dolphin)
				using winTimeFormat = std::chrono::duration<LONGLONG, std::ratio<100, std::nano::den>::type>;

				/*
				 * From Blat Blatnik's blog:
				 *
				 * "Also, [CreateWaitableTimerEx] has a quirk that if you request a sleep
				 * period longer than the system timer period, the precision of the timer plummets."
				 *
				 * We limit the sleep to 95% of the time period to avoid this quirk.
				 */
				constexpr auto kMaxTicks =
					duration_cast<winTimeFormat>(spinDuration) * 95 / 100;

				/*
				 * Loop using high precision sleep until end of frame is reached,
				 * to avoid oversleep and achieve high accuracy.
				 */
				while (true) {
					const auto timeNow = std::chrono::steady_clock::now();
					const auto sleepDuration = frameEnd - timeNow - spinDuration;

					// Limit maximum sleep duration to kMaxTicks (95% of 1 millisecond)
					const auto ticks = std::min(
						duration_cast<winTimeFormat>(sleepDuration), kMaxTicks
					).count();

					if (ticks <= 0)
						// Sleep time has finished, spin the rest
						break;

					// Negate ticks to make Windows use relative time
					const LARGE_INTEGER due_time{.QuadPart = -ticks};
					SetWaitableTimerEx(
						m_winTimerHandle, &timerDue, 0, nullptr,
						nullptr, nullptr, 0
					);

					// Wait for timer. Use INFINITE to disable timeout.
					WaitForSingleObject(m_winTimerHandle, INFINITE);
				}
			#else
				// sleep_until on Linux provides high accuracy
				const auto sleepPoint = frameEnd - spinDuration;

				std::this_thread::sleep_until(sleepPoint);
			#endif

			// Report oversleeps for debugging
			if (kDebugEnabled) {
				const auto timeNow = std::chrono::steady_clock::now();

				if (timeNow > frameEnd) {
					// The amount of microseconds overslept by
					const auto oversleepDuration =
						std::chrono::duration_cast<std::chrono::microseconds>(
							timeNow - frameEnd
						).count();

					std::cout <<
						"[DEBUG] Overslept! " << oversleepDuration << " microseconds late."
					<< std::endl;
				}
			}

			// Spin for the remaining time
			while (std::chrono::steady_clock::now() < frameEnd) {
				#if defined(_WIN32)
					YieldProcessor();
				#else
					std::this_thread::yield();
				#endif
			}
		} else if (kDebugEnabled) {
			// The amount of milliseconds the frame was late by
			const auto frameLag =
				std::chrono::duration_cast<std::chrono::milliseconds>(
					frameComplete - frameEnd
				).count();

			std::cout <<
				"[DEBUG] Slow frame! " << frameLag << " milliseconds late."
			<< std::endl;
		}
	}
}

Chip8::~Chip8() {
	#if defined(_WIN32)
		CloseHandle(m_winTimerHandle);

		// Restore timer resolution
		timeBeginPeriod(1);
	#endif
}