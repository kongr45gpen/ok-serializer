#pragma once

#include <bit>
#include <cstdint>

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
            constexpr static void serialize(const Value &v) = delete;
        };
    }

    template<int Bytes, end Endianness = end::be>
    requires (Bytes > 0 && Bytes <= 8)
    struct uint : public internal::type {
        template<typename V, class Output>
        constexpr static void serialize(const V &v, Output&& o) {
            if constexpr (Endianness == end::le) {
                for (std::size_t i = 0; i < Bytes; i++) {
                    o.add(static_cast<uint8_t>((v >> (8 * i)) & 0xFFU));
                }
            } else {
                for (std::size_t i = 0; i < Bytes; i++) {
                    o.add(static_cast<uint8_t>((v >> (8 * (Bytes - i - 1))) & 0xFFU));
                }
            }
        }
    };

    template<int Bytes, end Endianness = end::be>
    struct sint : public internal::type {
        template<typename V, class Output>
        static void serialize(const V &v, Output&& o) {
            using Unsigned = std::make_unsigned_t<V>;
            Unsigned u = std::bit_cast<Unsigned>(v);
            uint<Bytes, Endianness>::serialize(u, o);
        }
    };
}
