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
            m_data[m_topIndex] = element;
            m_topIndex = (m_topIndex + 1) % m_capacity;

            if (m_size < m_capacity)
                m_size++;
        }

        const T& operator[](std::uint16_t index) const {
            if (m_size == m_capacity)
                index = (m_topIndex + index) % m_capacity;

            return m_data[index];
        }

        [[nodiscard]] std::uint16_t size() const {
            return m_size;
        }

        void reduce(std::uint16_t amount) {
            m_size -= amount;
            m_topIndex = m_size;
        }

        void clear() {
            m_size = 0;
            m_topIndex = 0;
        }
};
