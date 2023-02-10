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
                for (uint8_t i = 0; i < Bytes; i++) {
                    o.add(static_cast<uint8_t>((v >> (8 * i)) & 0xFFU));
                }
            } else {
                for (uint8_t i = 0; i < Bytes; i++) {
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

    template<int Bytes = 4, end Endianness = end::be>
    requires (Bytes == 4 || Bytes == 8)
    struct floatp : public internal::type {
        template<typename V, class Output>
        requires (std::is_floating_point_v<V>)
        static void serialize(const V &v, Output&& o) {
            using Float = std::conditional_t<Bytes == 4, float, double>;
            using Unsigned = std::conditional_t<Bytes == 4, uint32_t, uint64_t>;

            Unsigned u = std::bit_cast<Unsigned>(static_cast<Float>(v));

            uint<Bytes, Endianness>::serialize(u, o);
        }
    };

    template<end Endianness = end::be>
    using doublep = floatp<8, Endianness>;

    template<typename Enum, int Bytes = sizeof(std::underlying_type<Enum>), end Endianness = end::be>
    requires (Bytes > 0 && Bytes <= 8 && std::is_enum_v<Enum>)
    struct enumv : public internal::type {
        template<class Output>
        static void serialize(const Enum &e, Output&& o) {
            using Underlying = std::underlying_type_t<Enum>;
            uint<Bytes, Endianness>::serialize(static_cast<Underlying>(e), o);
        }
    };
}
