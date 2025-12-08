#pragma once

#include <array>
#include <cstdint>
#include <iostream>

template<std::uint16_t m_size, bool debugEnabled>
class SafeArray {
    std::array<uint8_t, m_size> m_data{};

    public:
        inline uint8_t& operator[](uint16_t index) {
            // Exists solely to provide a useless reference
            // for an out-of-bounds access.
            static uint8_t dummy {0};

            if (index < m_size) {
                return m_data[index];
            } else if (debugEnabled) {
                // Print debug message for out-of-bounds access.
                std::cout
                    << "[ERROR] Out-of-bounds memory access at "
                    << index << " for size " << m_size
                    << std::endl;

            }

            /*
             * Out of bounds index into an array (operator[]) usually
             * segfaults, or for the .at() method it will throw an out_of_range
             * exception. In the case of emulation, we will assume it's an error in
             * the ROM and chose to continue execution rather than terminating.
             */
            return dummy;
        }

        // Allow .begin() method from std::array to be used for our SafeArray
        auto begin() {
            return m_data.begin();
        }
};
