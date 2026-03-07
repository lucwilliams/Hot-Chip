#include "Chip8.h"

/*
 * For opcodes where the most significant nibble is not unique,
 * this enum maps the least significant nibbles to their unique instructions.
 */
enum class opcode : std::uint8_t {
    CLEAR_DISPLAY = 0xE0,
    RETURN = 0xEE,
    REG_ASSIGNMENT = 0x0,
    REG_OR = 0x1,
    REG_AND = 0x2,
    REG_XOR = 0x3,
    REG_ADD = 0x4,
    REG_SUBTRACT = 0x5,
    REG_DIFFERENCE = 0x7,
    REG_LSHIFT = 0xE,
    REG_RSHIFT = 0x6,
    IS_KEY_PRESSED = 0x9E,
    IS_KEY_NOT_PRESSED = 0xA1,
    TIMER_GET_DELAY = 0x07,
    TIMER_DELAY_SET = 0x15,
    TIMER_SOUND_SET = 0x18,
    AWAIT_KEY = 0x0A,
    ADD_TO_I = 0x1E,
    LOAD_CHAR = 0x29,
    BCD_VX = 0x33,
    DUMP_REG = 0x55,
    LOAD_REG = 0x65
};

void Chip8::opcode0(std::uint16_t instruction) {
    const opcode lowByte = static_cast<opcode>(
        getLowByte(instruction)
    );

	switch (lowByte) {
        case opcode::CLEAR_DISPLAY:
            m_window.clearDisplay();
	        m_window.pushInstructionHistory("DISPLAY CLEAR");
            break;
	    case opcode::RETURN: {
	        const std::uint16_t preReturnAddress{m_PC};

            if (m_stackSize > 0) {
                // Pop return address from the stack
                m_PC = m_stack[--m_stackSize];
                m_PCUpdated = true;
            } else {
                if (kDebugEnabled)
                    std::cout <<
                        "[ERROR] Return attempted from outside of subroutine. "
                        << instruction << std::endl;
            }

	        m_window.pushInstructionHistory(
	            std::format("RETURN {} -> {}",
	                u16toHex(preReturnAddress), u16toHex(m_PC)
	            )
	        );

            break;
        }
        default:
            if (kDebugEnabled)
                std::cout << "[DEBUG] Unknown instruction: " << instruction << std::endl;
    }
}

// GOTO NNN
void Chip8::opcode1(std::uint16_t instruction) {
    // Update PC to new address from instruction
    m_PC = getAddressFromInstruction(instruction);
    m_PCUpdated = true;

	m_window.pushInstructionHistory(std::format("GOTO {}", u16toHex(instruction)));
}

// CALL NNN
void Chip8::opcode2(std::uint16_t instruction) {
    // Prevent out-of-bounds stack access.
    // Stack limited to a maximum subroutine depth of 16.
    if (m_stackSize < 16) {
        // Push return address to stack (next instruction)
        m_stack[m_stackSize++] = m_PC + 2;

        // Update PC to new address from instruction
        m_PC = getAddressFromInstruction(instruction);
        m_PCUpdated = true;
    } else {
        // Output error and continue execution without calling subroutine
        std::cout <<
            "[ERROR] Maximum stack depth exceeded!"
            << instruction << std::endl;
    }

	m_window.pushInstructionHistory(std::format("CALL {}", u16toHex(m_PC)));
}

// if (Vx == NN)
void Chip8::opcode3(std::uint16_t instruction) {
    // Get register value
    std::uint8_t regIndex = nibbleAt(instruction, 2);
    std::uint8_t VX = m_registers[regIndex];

    // Get 8-bit constant
    std::uint8_t NN = getLowByte(instruction);

    if (VX == NN)
        m_PC += 2;
    
	m_window.pushInstructionHistory(
	    std::format("IF V{} == {}", u8toHex(regIndex), u8toHex(NN))
	);
}

// if (Vx != NN)
void Chip8::opcode4(std::uint16_t instruction) {
    // Get register value
    std::uint8_t regIndex = nibbleAt(instruction, 2);
    std::uint8_t VX = m_registers[regIndex];

    // Get 8-bit constant
    std::uint8_t NN = getLowByte(instruction);

    if (VX != NN)
        m_PC += 2;
    
    m_window.pushInstructionHistory(
        std::format("IF V{} != {}", u8toHex(regIndex), u8toHex(NN))
    );
}

// if (Vx == Vy)
void Chip8::opcode5(std::uint16_t instruction) {
    // Get register indexes
    std::uint8_t VX_index = nibbleAt(instruction, 2);
    std::uint8_t VY_index = nibbleAt(instruction, 1);
    
    // Get register value
    std::uint8_t VX = m_registers[VX_index];
    std::uint8_t VY = m_registers[VY_index];

    if (VX == VY)
        m_PC += 2;
    
    m_window.pushInstructionHistory(
        std::format("IF V{} == V{}", u8toHex(VX_index), u8toHex(VY_index))
    );
}

// VX = NN
void Chip8::opcode6(std::uint16_t instruction) {
    // Get register number
    std::uint8_t VX = nibbleAt(instruction, 2);

    // Get 8-bit constant
    std::uint8_t NN = getLowByte(instruction);

    // Set VX = NN
    m_registers[VX] = NN;

    m_window.pushInstructionHistory(
        std::format("V{} = {}", u8toHex(VX), u8toHex(NN))
    );
}

// VX += NN
void Chip8::opcode7(std::uint16_t instruction) {
    // Get register number
    std::uint8_t VX = nibbleAt(instruction, 2);

    // Get 8-bit constant
    std::uint8_t NN = getLowByte(instruction);

    // Add NN to VX
    m_registers[VX] += NN;

    m_window.pushInstructionHistory(
        std::format("V{} += {}", u8toHex(VX), u8toHex(NN))
    );
}


// Register arithmetic and bitwise operations
void Chip8::opcode8(std::uint16_t instruction) {
    const opcode lastNibble = static_cast<opcode>(
        nibbleAt(instruction, 0)
    );

    // Get register indexes
    std::uint8_t VX_index = nibbleAt(instruction, 2);
    std::uint8_t VY_index = nibbleAt(instruction, 1);

    std::uint8_t& VX = m_registers[VX_index];
    std::uint8_t& VY = m_registers[VY_index];
    std::uint8_t& VF = m_registers[0xF];

    switch (lastNibble) {
        case opcode::REG_ASSIGNMENT:
            VX = VY;

            m_window.pushInstructionHistory(
                std::format("V{} = V{}", u8toHex(VX_index), u8toHex(VY_index))
            );
            break;
        case opcode::REG_OR:
            VX |= VY;

            m_window.pushInstructionHistory(
                std::format("V{} |= V{}", u8toHex(VX_index), u8toHex(VY_index))
            );
            break;
        case opcode::REG_AND:
            VX &= VY;

            m_window.pushInstructionHistory(
                std::format("V{} &= V{}", u8toHex(VX_index), u8toHex(VY_index))
            );
            break;
        case opcode::REG_XOR:
            VX ^= VY;

            m_window.pushInstructionHistory(
                std::format("V{} ^= V{}", u8toHex(VX_index), u8toHex(VY_index))
            );
            break;
        case opcode::REG_ADD:
        {
            // Use a 16-bit unsigned int to get the sum without overflow
            std::uint16_t sum = VX + VY;

            VX = sum;

            // Does the sum of the two registers exceed the maximum for std::uint8_t?
            if (sum > std::numeric_limits<std::uint8_t>::max()) {
                VF = 1;
            } else {
                VF = 0;
            }

            m_window.pushInstructionHistory(
                std::format(
                    "V{} += V{}, VF = {}",
                    u8toHex(VX_index), u8toHex(VY_index), u8toHex(VF)
                )
            );
            break;
        }
        case opcode::REG_SUBTRACT:
        {
            bool underflow = (VX < VY);
            VX -= VY;

            // Does the difference result in an underflow?
            if (underflow) {
                VF = 0;
            } else {
                VF = 1;
            }

            m_window.pushInstructionHistory(
                std::format(
                    "V{} -= V{}, VF = {}",
                    u8toHex(VX_index), u8toHex(VY_index), u8toHex(VF)
                )
            );
            break;
        }
        case opcode::REG_DIFFERENCE:
        {
            bool underflow = (VY < VX);
            VX = VY - VX;

            // Does the difference result in an underflow?
            if (underflow) {
                VF = 0;
            } else {
                VF = 1;
            }

            m_window.pushInstructionHistory(
                std::format(
                    "V{} = V{} - V{}, VF = {}",
                    u8toHex(VX_index), u8toHex(VY_index), u8toHex(VX_index), u8toHex(VF)
                )
            );
            break;
        }
        case opcode::REG_LSHIFT: {
            // QUIRK
            VX = VY;

            std::uint8_t VX_MSB = (VX & kMSBMask) >> 7;
            VX <<= 1;

            // Store MSB of VX in VF
            VF = VX_MSB;

            m_window.pushInstructionHistory(
                std::format(
                    "V{} = V{} << 1, VF = {}",
                    u8toHex(VX_index), u8toHex(VY_index), u8toHex(VF)
                )
            );
            break;
        }
        case opcode::REG_RSHIFT: {
            // QUIRK
            VX = VY;

            std::uint8_t VX_LSB = VX & 1;
            VX >>= 1;

            // Store LSB of VX in VF
            VF = VX_LSB;

            m_window.pushInstructionHistory(
                std::format(
                    "V{} = V{} >> 1, VF = {}",
                    u8toHex(VX_index), u8toHex(VY_index), u8toHex(VF)
                )
            );
            break;
        }
        default:
            if (kDebugEnabled)
                std::cout << "[DEBUG] Unknown instruction: " << instruction << std::endl;
    }
}

// if (Vx != Vy)
void Chip8::opcode9(std::uint16_t instruction) {
    // Get register indexes
    std::uint8_t VX_index = nibbleAt(instruction, 2);
    std::uint8_t VY_index = nibbleAt(instruction, 1);

    // Get register value
    std::uint8_t VX = m_registers[VX_index];
    std::uint8_t VY = m_registers[VY_index];


    if (VX != VY)
        m_PC += 2;

    m_window.pushInstructionHistory(
        std::format("IF V{} != V{}", u8toHex(VX_index), u8toHex(VY_index))
    );
}

// I = NNN
void Chip8::opcodeA(std::uint16_t instruction) {
    // Get 12 byte address
    std::uint16_t NNN = getAddressFromInstruction(instruction);

    // Set I to NNN
    m_index = NNN;

    m_window.pushInstructionHistory(
        std::format("I = {}", u16toHex(NNN))
    );
}

// PC = V0 + NNN
void Chip8::opcodeB(std::uint16_t instruction) {
    std::uint8_t V0 = m_registers[0];
    std::uint16_t NNN = getAddressFromInstruction(instruction);

    // Set PC to V0 + NNN
    m_PC = V0 + NNN;
    m_PCUpdated = true;

    m_window.pushInstructionHistory(
        std::format("PC = {} + {}", u8toHex(V0), u16toHex(NNN))
    );
}

// VX = rand(0, NN), NN < 256
void Chip8::opcodeC(std::uint16_t instruction) {
    std::uint8_t regIndex = nibbleAt(instruction, 2);
    std::uint8_t& VX = m_registers[regIndex];
    std::uint8_t NN = getLowByte(instruction);

    VX = m_randUint8(m_mersenneTwister) % NN;

    m_window.pushInstructionHistory(
        std::format("V{} = {} (RAND)", u8toHex(regIndex), u8toHex(VX))
    );
}

// draw(Vx, Vy, N)
void Chip8::opcodeD(std::uint16_t instruction) {
    bool bitFlipped = false;

    std::uint8_t VX = m_registers[nibbleAt(instruction, 2)];
    std::uint8_t VY = m_registers[nibbleAt(instruction, 1)];
    std::uint8_t& VF = m_registers[0xF];

    std::uint8_t height = nibbleAt(instruction, 0);

    // Draw each row for height N
    for (std::uint8_t y {0}; y < height; ++y) {
        std::uint8_t rowData = m_memory[m_index + y];

        // Update the framebuffer to draw this rowData of pixels
        bool rowBitFlip = m_window.drawRow(
            VX, VY + y, rowData
        );

        // True if a bit has been flipped in this rowData or a prior rowData.
        bitFlipped = rowBitFlip || bitFlipped;
    }

    // VF is set to 1 if any screen pixels are flipped from set to unset
    // when the sprite is drawn, and to 0 if that does not happen.
    if (bitFlipped) {
        VF = 1;
    } else {
        VF = 0;
    }

    m_window.pushInstructionHistory(
        std::format(
            "DRAW: ({}, {}), N: {}, VF: ",
            u8toHex(VX), u8toHex(VY), u8toHex(height), u8toHex(VF)
        )
    );
}

// Skip next instruction if key stored in VX is pressed
void Chip8::opcodeE(std::uint16_t instruction) {
    const opcode lowByte = static_cast<opcode>(
        getLowByte(instruction)
    );

    std::uint8_t VX = m_registers[nibbleAt(instruction, 2)];

    switch (lowByte) {
        case opcode::IS_KEY_PRESSED:
            if (m_keyStates[VX])
                // Instructions are two bytes, increment by two
                m_PC += 2;

            m_window.pushInstructionHistory(
                std::format("SKIP IF {} PRESSED", u8toHex(VX))
            );
            break;
        case opcode::IS_KEY_NOT_PRESSED:
            if (!m_keyStates[VX])
                m_PC += 2;

            m_window.pushInstructionHistory(
                std::format("SKIP IF {} NOT PRESSED", u8toHex(VX))
            );
            break;
        default:
            if (kDebugEnabled)
                std::cout << "[DEBUG] Unknown instruction: " << instruction << std::endl;
    }
}

// Timers, keystrokes and misc memory instructions involving VX
void Chip8::opcodeF(std::uint16_t instruction) {
    const opcode lowByte = static_cast<opcode>(
        getLowByte(instruction)
    );

    std::uint8_t VX_index = nibbleAt(instruction, 2);
    std::uint8_t& VX = m_registers[VX_index];

    switch (lowByte) {
        case opcode::TIMER_GET_DELAY:
            VX = m_delayTimer.readTimer();

            m_window.pushInstructionHistory(
                std::format("GET DELAY TIMER : {}", u8toHex(VX))
            );
            break;
        case opcode::TIMER_DELAY_SET:
            m_delayTimer.setTimer(VX);

            m_window.pushInstructionHistory(
               std::format("SET DELAY TIMER: {}", u8toHex(VX))
            );
            break;
        case opcode::TIMER_SOUND_SET:
            m_window.pushInstructionHistory(
              std::format("SET SOUND TIMER: {}", u8toHex(VX))
            );

            m_soundTimer.setTimer(VX);
            break;
        case opcode::AWAIT_KEY:
            // Behaviour of this instruction takes place in the main event loop in executionLoop()
            m_awaitingKey = true;

            m_window.pushInstructionHistory("AWAITING KEYPRESS");
            break;
        case opcode::ADD_TO_I:
            m_index += VX;

            m_window.pushInstructionHistory(
              std::format("I += {}, I = {}", u8toHex(VX), u8toHex(m_index))
            );
            break;
        case opcode::LOAD_CHAR:
            // Each font consists of five bytes.
            m_index = m_memory[kFontOffset + (VX * 5)];

            m_window.pushInstructionHistory(
                std::format("LOAD CHAR: {}", u8toHex(VX))
            );
            break;
        case opcode::BCD_VX:
            // Express VX's value in BCD format (hundreds, tens, ones)
            m_memory[m_index] = VX / 100;
            m_memory[m_index + 1] = (VX % 100) / 10;
            m_memory[m_index + 2] = VX % 10;
        
            m_window.pushInstructionHistory(
                std::format(
                    "BCD V{} = {} INDEX: {}",
                    u8toHex(VX_index), u8toHex(VX), u8toHex(m_index)
                )
            );
            break;
        case opcode::DUMP_REG:
            // Store the value of all registers up to VX, starting at the address of I
            for (std::uint8_t x = 0; x <= VX_index; ++x) {
                m_memory[m_index + x] = m_registers[x];
            }

            m_window.pushInstructionHistory(
                std::format(
                    "DUMP REG: VX = {}, I = {}",
                    u8toHex(VX_index), u8toHex(m_index)
                )
            );
            break;
        case opcode::LOAD_REG:
            // Load the values starting at the address of I into registers up to VX
            for (std::uint8_t x = 0; x <= VX_index; ++x) {
                m_registers[x] = m_memory[m_index + x];
            }

            m_window.pushInstructionHistory(
                std::format(
                    "LOAD REG: VX = {}, I = {}",
                    u8toHex(VX_index), u8toHex(m_index)
                )
            );
            break;
        default:
            if (kDebugEnabled)
                std::cout << "[DEBUG] Unknown instruction: " << instruction << std::endl;
    }
}
