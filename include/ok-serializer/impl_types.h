#pragma once

#include <bit>
#include <cstdint>

namespace okser {
    /**
     * Enum to represent endianness
     *
     * @note This library does not depend on the endianness of the compiler or target system. Any output
     * should be interchangeable between different systems.
     *
     * However, the user can specify the endianness of the serialized data at compile-time. Big endianness
     * is used conventionally by default.
     *
     * @see https://en.wikipedia.org/wiki/Endianness
     * @todo Use standard library definition, https://en.cppreference.com/w/cpp/types/endian
     */
    enum class end {
        le, ///< Little endian
        be, ///< Big endian
    };

    namespace internal {
        /**
         * A generic base class for serializer types. Does not provide any useful functionality.
         */
        class type {};

        /**
         * A concept to check if a class is a serializer.
         * @internal
         */
        template<class T>
        constexpr inline bool is_serializer = false;

        /**
         * Specialization to consider all children of okser::internal::type as serializers.
         */
        template<class T>
        requires std::derived_from<T, type>
        constexpr inline bool is_serializer<T> = true;
    }

    /**
     * Unsigned integer type
     * @tparam Bytes The number of bytes in the binary representation of the integer
     * @tparam Endianness
     */
    template<int Bytes, end Endianness = end::be>
    requires (Bytes > 0 && Bytes <= 8)
    struct uint : public internal::type {
        template<typename V, Output Out>
        constexpr static void serialize(const V &v, Out&& o) {
            if constexpr (Endianness == end::le) {
                for (int i = 0; i < Bytes; i++) {
                    o.add(static_cast<uint8_t>((v >> (8 * i)) & 0xFFU));
                }
            } else {
                for (int i = 0; i < Bytes; i++) {
                    o.add(static_cast<uint8_t>((v >> (8 * (Bytes - i - 1))) & 0xFFU));
                }
            }
        }

        template<typename V, Input In>
        constexpr static std::pair<V,In> deserialize(In in) {
            V result = 0;
            In input = in;
            if constexpr (Endianness == end::le) {
                for (int i = 0; i < Bytes; i++) {
                    std::optional<uint8_t> value;
                    std::tie(value, input) = input.get();
                    result |= static_cast<V>(value.value()) << (8 * i);
                }
            } else {
                for (int i = 0; i < Bytes; i++) {
                    std::optional<uint8_t> value;
                    std::tie(value, input) = input.get();
                    result |= static_cast<V>(value.value()) << (8 * (Bytes - i - 1));
                }
            }
            return {result, input};
        }
    };

    /**
     * Signed integer type
     *
     * Implemented using 2's complement
     *
     * @tparam Bytes The number of bytes in the binary representation of the integer
     * @tparam Endianness
     */
    template<int Bytes, end Endianness = end::be>
    struct sint : public internal::type {
        template<typename V, Output Out>
        static void serialize(const V &v, Out&& o) {
            using Unsigned = std::make_unsigned_t<V>;
            Unsigned u = std::bit_cast<Unsigned>(v);
            uint<Bytes, Endianness>::serialize(u, o);
        }

        template<typename V, Input In>
        static std::pair<V, In> deserialize(In in) {
            using Unsigned = std::make_unsigned_t<V>;
            Unsigned u;

            std::tie(u, in) = uint<Bytes, Endianness>::template deserialize<Unsigned>(in);
            return {std::bit_cast<V>(u), in};
        }
    };

    /**
     * IEEE 754 floating point number
     * @tparam Bytes The number of bytes in the binary representation of the number,
     *               either 4 (single-precision, equivalent to C's `float`), or 8
     *               (double-precision, equivalent to C's `double`)
     * @tparam Endianness
     */
    template<int Bytes = 4, end Endianness = end::be>
    requires (Bytes == 4 || Bytes == 8)
    struct floatp : public internal::type {
        template<typename V, Output Out>
        requires (std::is_floating_point_v<V>)
        static void serialize(const V &v, Out&& o) {
            using Float = std::conditional_t<Bytes == 4, float, double>;
            using Unsigned = std::conditional_t<Bytes == 4, uint32_t, uint64_t>;

            Unsigned u = std::bit_cast<Unsigned>(static_cast<Float>(v));

            uint<Bytes, Endianness>::serialize(u, o);
        }

        template<typename V = std::conditional_t<Bytes == 4, float, double>, Input In>
        requires (std::is_floating_point_v<V>)
        static std::pair<V, In> deserialize(In&& in) {
            using Float = std::conditional_t<Bytes == 4, float, double>;
            using Unsigned = std::conditional_t<Bytes == 4, uint32_t, uint64_t>;

            Unsigned u;
            std::tie(u, in) = uint<Bytes, Endianness>::template deserialize<Unsigned>(in);
            return {std::bit_cast<Float>(u), in};
        }
    };

    /**
     * Shortcut to a double-precision floating point number
     */
    template<end Endianness = end::be>
    using doublep = floatp<8, Endianness>;

    /**
     * Enumeration value
     *
     * This class provides a convenient translation from an enum to an integer, without the need to
     * mess with casts and conversions.
     *
     * The enumeration value is first converted to its scalar underlying type, and then serialized as
     * a normal number.
     *
     * @todo Check what happens with negative enum values
     * @todo Check what happens when bytes < sizeof(Enum)
     *
     * @tparam Enum The enumeration type
     * @tparam Bytes Number of bytes to use for the representation. Defaults to the size of the underlying
     *               type of the Enum.
     * @tparam Endianness
     */
    template<typename Enum, int Bytes = sizeof(std::underlying_type<Enum>), end Endianness = end::be>
    requires (Bytes > 0 && Bytes <= 8 && std::is_enum_v<Enum>)
    struct enumv : public internal::type {
        template<Output Out>
        static void serialize(const Enum &e, Out&& o) {
            using Underlying = std::underlying_type_t<Enum>;
            uint<Bytes, Endianness>::serialize(static_cast<Underlying>(e), o);
        }

        template<Input In>
        static std::pair<Enum, In> deserialize(In in) {
            using Underlying = std::underlying_type_t<Enum>;

            auto [result, input] = uint<Bytes, Endianness>::template deserialize<Underlying>(in);

            return {static_cast<Enum>(result), input};
        }
    };
}
