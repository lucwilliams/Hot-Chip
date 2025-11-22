#include "Chip8.h"

// For opcodes where the most significant nibble is not unique,
// this enum maps the least significant nibbles to their unique instructions.
enum class opcode : uint8_t {
    CLEAR_DISPLAY = 0xE0,
    RETURN = 0xEE,
    REG_ASSIGNMENT = 0x0,
    REG_OR = 0x1,
    REG_AND = 0x2,
    REG_XOR = 0x3,
    REG_ADD = 0x4,
    REG_SUBTRACT = 0x5,
    REG_RSHIFT = 0x6,
    REG_DIFFERENCE = 0x7,
    REG_LSHIFT = 0xE
};

void Chip8::opcode0(uint16_t instruction) {
    const opcode lowByte = static_cast<opcode>(
        getLowByte(instruction)
    );

	switch (lowByte) {
        case opcode::CLEAR_DISPLAY:
            if (m_debug)
                std::cout << "[DEBUG] clearDisplay() executed. (instruction 0x00E0)" << std::endl;

            m_window.clearDisplay();
            break;
        case opcode::RETURN:
            // Pop return address from the stack
            m_PC = m_stack[--m_stackSize];
            m_PCUpdated = true;
            break;
        default:
            break;
    }
}

// GOTO NNN
void Chip8::opcode1(uint16_t instruction) {
    // Update PC to new address from instruction
    m_PC = getAddressFromInstruction(instruction);
    m_PCUpdated = true;
}

// CALL *NNN
void Chip8::opcode2(uint16_t instruction) {
    // Push return address to stack (next instruction)
    m_stack[m_stackSize++] = m_PC + 2;

    // Get pointer to subroutine address
    uint16_t NNN = getAddressFromInstruction(instruction);

    // Jump to dereferenced pointer
    m_PC = m_memory[NNN];
    m_PCUpdated = true;
}

// if (Vx == NN)
void Chip8::opcode3(uint16_t instruction) {
    // Get register value
    uint8_t VX = m_registers[nibbleAt(instruction, 2)];

    // Get 8-bit constant
    uint8_t NN = getLowByte(instruction);

    if (VX == NN)
        m_PC += 2;
}

// if (Vx != NN)
void Chip8::opcode4(uint16_t instruction) {
    // Get register value
    uint8_t VX = m_registers[nibbleAt(instruction, 2)];

    // Get 8-bit constant
    uint8_t NN = getLowByte(instruction);

    if (VX != NN)
        m_PC += 2;
}

// if (Vx == Vy)
void Chip8::opcode5(uint16_t instruction) {
    // Get register value
    uint8_t VX = m_registers[nibbleAt(instruction, 2)];
    uint8_t VY = m_registers[nibbleAt(instruction, 1)];

    if (VX == VY)
        m_PC += 2;
}

// VX = NN
void Chip8::opcode6(uint16_t instruction) {
    // Get register number
    uint8_t VX = nibbleAt(instruction, 2);

    // Get 8-bit constant
    uint8_t NN = getLowByte(instruction);

    // Set VX = NN
    m_registers[VX] = NN;
}

// VX += NN
void Chip8::opcode7(uint16_t instruction) {
    // Get register number
    uint8_t VX = nibbleAt(instruction, 2);

    // Get 8-bit constant
    uint8_t NN = getLowByte(instruction);

    // Add NN to VX
    m_registers[VX] += NN;
}


/*
 * REG_ASSIGNMENT = 0x0,
 * REG_OR = 0x1,
 * REG_AND = 0x2,
 * REG_XOR = 0x3,
 * REG_ADD = 0x4,
 * REG_SUBTRACT = 0x5,
 * REG_RSHIFT = 0x6,
 * REG_DIFFERENCE = 0x7,
 * REG_LSHIFT = 0xE
*/
void Chip8::opcode8(uint16_t instruction) {
    const opcode lastNibble = static_cast<opcode>(
        nibbleAt(instruction, 0)
    );

    uint8_t VY = nibbleAt(instruction, 1);
    uint8_t VX = nibbleAt(instruction, 2);

    switch (lastNibble) {
        case opcode::REG_ASSIGNMENT:
            m_registers[VX] = m_registers[VY];
            break;
        case opcode::REG_OR:
            m_registers[VX] |= m_registers[VY];
            break;
        case opcode::REG_AND:
            m_registers[VX] &= m_registers[VY];
            break;
        case opcode::REG_XOR:
            m_registers[VX] ^= m_registers[VY];
            break;
        case opcode::REG_ADD:
        {
            // Use a 16-bit unsigned int to get the sum without overflow
            uint16_t sum = m_registers[VX] + m_registers[VY];

            // Does the sum of the two registers exceed the maximum for uint8_t?
            if (sum > std::numeric_limits<uint8_t>::max()) {
                m_registers[0xF] = 1;
            } else {
                m_registers[0xF] = 0;
            }

            m_registers[VX] += m_registers[VY];
        }

        case opcode::REG_SUBTRACT:break;
        case opcode::REG_RSHIFT:break;
        case opcode::REG_DIFFERENCE:break;
        case opcode::REG_LSHIFT:break;
        default:
            break;
    }
}

// if (Vx != Vy)
void Chip8::opcode9(uint16_t instruction) {
    // Get register value
    uint8_t VX = m_registers[nibbleAt(instruction, 2)];
    uint8_t VY = m_registers[nibbleAt(instruction, 1)];

    if (VX != VY)
        m_PC += 2;
}

// I = NNN
void Chip8::opcodeA(uint16_t instruction) {
    // Get 12 byte address
    uint16_t NNN = getAddressFromInstruction(instruction);

    // Set I to NNN
    m_index = NNN;
}

// PC = V0 + NNN
void Chip8::opcodeB(uint16_t instruction) {
    uint8_t V0 = m_registers[0];
    uint16_t NNN = getAddressFromInstruction(instruction);

    // Set I to V0 + NNN
    m_index = V0 + NNN;
}

// draw(Vx, Vy, N)
void Chip8::opcodeD(uint16_t instruction) {
    bool bitFlipped = false;

    uint8_t VX = m_registers[nibbleAt(instruction, 2)];
    uint8_t VY = m_registers[nibbleAt(instruction, 1)];
    uint8_t height = nibbleAt(instruction, 0);

    // Draw each row for height N
    for (uint8_t y {0}; y < height; ++y) {
        uint8_t rowData = static_cast<uint8_t>(
            m_memory[m_index + y]
        );

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
        m_registers[0xF] = 1;
    } else {
        m_registers[0xF] = 0;
    }
}
