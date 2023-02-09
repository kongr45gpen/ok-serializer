#pragma once

namespace okser {
    /**
     * Enum to represent endianness
     */
    enum class end {
        le, ///< Little endian
        be, ///< Big endian
    };

    namespace internal {
        class type {
            template<typename Value>
            static std::string serialize(const Value &v) = delete;
        };
    }

    template<int Bytes, end Endianness = end::be>
    struct sint : public internal::type {
        static std::string serialize(const uint32_t &v) {
            std::array<char, Bytes> arr;
            for (int i = 0; i < Bytes; i++) {
                arr[i] = (v >> (8 * i)) & 0xFFU;
            }
            return std::string(arr.data(), Bytes);
        }
    };

    template<int Bytes, end Endianness = end::be>
    struct uint : public internal::type {
        static std::string serialize(const uint32_t &v) {
            std::array<char, Bytes> arr;
            for (int i = 0; i < Bytes; i++) {
                arr[i] = (v >> (8 * i)) & 0xFFU;
            }
            return std::string(arr.data(), Bytes);
        }
    };

}
