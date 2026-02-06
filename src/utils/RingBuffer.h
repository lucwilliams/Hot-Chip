#pragma once

#include <array>
#include <cstdint>

template<typename T, std::uint16_t m_capacity>
class RingBuffer {
    std::array<T, m_capacity> m_data{};
    std::uint16_t m_size{0};
    std::uint16_t m_topIndex{0};

    public:
        void push(const T& element) {
            m_data[m_topIndex % m_capacity] = element;
            ++m_topIndex;

            if (m_size < m_capacity)
                m_size++;
        }

        const T& operator[](std::uint16_t index) const {
            return m_data[index];
        }

        std::uint16_t size() {
            return m_size;
        }

        void reduce(std::uint16_t amount) {
            m_size -= amount;
            m_topIndex = m_size;
        }

        void clear() {
            m_data.fill(0);
            m_size = 0;
        }
};
