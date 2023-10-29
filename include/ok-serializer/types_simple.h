#pragma once

#include <bit>
#include <cstdint>
#include "concepts.h"
#include "utils_early.h"
#include "context.h"

/**
 * @defgroup simple_types Simple Types
 */

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
class type {
};
}

/**
 * Unsigned integer type
 * @ingroup simple_types
 * @tparam Bytes The number of bytes in the binary representation of the integer
 * @tparam Endianness
 */
template<int Bytes, end Endianness = end::be> requires (Bytes > 0 && Bytes <= 8)
struct uint : public internal::type {
    using DefaultType = okser::internal::uint_bytes_to_type<Bytes>;

    template<typename V, OutputContext Context>
    constexpr static empty_result serialize(Context &&out, const V &v) {
        if constexpr (Endianness == end::le) {
            for (int i = 0; i < Bytes; i++) {
                auto result = out->add(static_cast<uint8_t>((v >> (8 * i)) & 0xFFU));

                if (!result) {
                    return result;
                }
            }
        } else {
            for (int i = 0; i < Bytes; i++) {
                auto result = out->add(static_cast<uint8_t>((v >> (8 * (Bytes - i - 1))) & 0xFFU));

                if (!result) {
                    return result;
                }
            }
        }

        return {};
    }

    template<typename V, InputContext Context>
    constexpr static okser::result<V> deserialize(Context &context) {
        auto result = context->template get<Bytes>();

        if (!result) {
            return std::unexpected(result.error());
        }

        V value = 0;
        if constexpr (Endianness == end::le) {
            for (int i = 0; i < Bytes; i++) {
                value |= static_cast<V>(result.value()[i]) << (8 * i);
            }
        } else {
            for (int i = 0; i < Bytes; i++) {
                value |= static_cast<V>(result.value()[i]) << (8 * (Bytes - i - 1));
            }
        }

        return value;
    }
};

/**
 * Signed integer type
 *
 * Implemented using 2's complement
 *
 * @ingroup simple_types
 * @tparam Bytes The number of bytes in the binary representation of the integer
 * @tparam Endianness
 */
template<int Bytes, end Endianness = end::be>
struct sint : public internal::type {
    using DefaultType = std::make_signed_t<okser::internal::uint_bytes_to_type<Bytes>>;

    template<typename V, OutputContext Context>
    static empty_result serialize(Context &&out, const V &v) {
        using Unsigned = std::make_unsigned_t<V>;
        Unsigned u = std::bit_cast<Unsigned>(v);
        return uint<Bytes, Endianness>::serialize(out, u);
    }

    template<typename V, InputContext Context>
    static okser::result<V> deserialize(Context &context) {
        using Unsigned = std::make_unsigned_t<V>;

        auto value = uint<Bytes, Endianness>::template deserialize<Unsigned>(context);
        value.transform(std::bit_cast<V, Unsigned>);

        return value;
    }
};

/**
 * IEEE 754 floating point number
 * @ingroup simple_types
 * @tparam Bytes The number of bytes in the binary representation of the number,
 *               either 4 (single-precision, equivalent to C's `float`), or 8
 *               (double-precision, equivalent to C's `double`)
 * @tparam Endianness
 */
template<int Bytes = 4, end Endianness = end::be> requires (Bytes == 4 || Bytes == 8)
struct floatp : public internal::type {
    using DefaultType = std::conditional_t<Bytes == 4, float, double>;

    template<typename V, OutputContext Context>
    requires(std::is_floating_point_v<V>)
    static empty_result serialize(Context &&out, const V &v) {
        using Unsigned = std::conditional_t<Bytes == 4, uint32_t, uint64_t>;

        Unsigned u = std::bit_cast<Unsigned>(static_cast<DefaultType>(v));

        return uint<Bytes, Endianness>::serialize(out, u);
    }

    template<typename V = DefaultType, InputContext Context>
    requires(std::is_floating_point_v<V>)
    static constexpr okser::result<V> deserialize(Context &context) {
        using Unsigned = std::conditional_t<Bytes == 4, uint32_t, uint64_t>;

        auto value = uint<Bytes, Endianness>::template deserialize<Unsigned>(context);

        return value.transform(std::bit_cast<DefaultType, Unsigned>);
    }
};

/**
 * Shortcut to a double-precision floating point number
 */
template<end Endianness = end::be>
using doublep = floatp<8, Endianness>;

/**
 * A variable-width integer (varint) as specified by Google's Protocol Buffers
 *
 * Variable-width integers allow encoding integers of arbitrary size, with small values using fewer bytes.
 * Each byte encodes 7 bits of the number, and the 8th bit is used to indicate whether the number continues.
 *
 * See https://protobuf.dev/programming-guides/encoding/#varints for implementation details.
 *
 * Varints are always encoded with big endianness.
 *
 * @ingroup simple_types
 * @todo Implement signed protobuf implementation
 */
struct varint : public internal::type {
    using DefaultType = uint64_t;

    template<typename V, OutputContext Context>
    requires(std::is_integral_v<V> && std::is_unsigned_v<V> && sizeof(V) <= 8)
    static constexpr empty_result serialize(Context &&out, V v) {
        for (int i = 0; i < 10; i++) {
            auto byte = static_cast<uint8_t>(v & 0b0111'1111);
            v >>= 7;

            if (v != 0) {
                byte |= 0b1000'0000;
            }

            if (auto result = out->add(byte); !result) {
                return result;
            }

            if (v == 0) {
                break;
            }
        }

        return {};
    }

    template<typename V = DefaultType, InputContext Context>
    requires(std::is_integral_v<V> && std::is_unsigned_v<V> && sizeof(V) <= 8)
    static constexpr okser::result<V> deserialize(Context &context) {
        V value = 0;

        // Must keep an upper bound on number of bytes read to prevent reading out-of-bounds
        for (uint32_t i = 0; i < 10; i++) {
            auto byte = context->get();

            if (!byte) {
                return std::unexpected(byte.error());
            }

            if (i == 9 && (*byte & 0b1000'0000) != 0) [[unlikely]] {
                return std::unexpected(okser::error_type::malformed_input);
            }

            uint8_t clear_byte = *byte & 0b0111'1111;

            if (std::bit_width(clear_byte) + i * 7 > std::numeric_limits<V>::digits) {
                return std::unexpected(okser::error_type::overflow);
            }

            value |= static_cast<V>(clear_byte) << (7 * i);

            if (!(*byte & 0b1000'0000)) {
                break;
            }
        }

        return value;
    }
};

/**
 * Enumeration value
 *
 * This class provides a convenient translation from an enum to an integer, without the need to
 * mess with casts and conversions.
 *
 * The enumeration value is first converted to its scalar underlying type, and then serialized as
 * a normal number.
 *
 * @ingroup simple_types
 * @todo Check what happens with negative enum values
 * @todo Check what happens when bytes < sizeof(Enum)
 *
 * @tparam Enum The enumeration type
 * @tparam Bytes Number of bytes to use for the representation. Defaults to the size of the underlying
 *               type of the Enum.
 * @tparam Endianness
 */
template<typename Enum, int Bytes = sizeof(std::underlying_type<Enum>), end Endianness = end::be> requires (Bytes > 0 &&
                                                                                                            Bytes <=
                                                                                                                    8 &&
                                                                                                            std::is_enum_v<Enum>)
struct enumv : public internal::type {
protected:
    using Underlying = std::underlying_type_t<Enum>;
public:
    using DefaultType = Enum;

    template<OutputContext Context>
    static empty_result serialize(Context &&out, const Enum &e) {
        return uint<Bytes, Endianness>::serialize(out, static_cast<Underlying>(e));
    }

    template<typename E = Enum, InputContext Context>
    requires(std::is_same_v<E, Enum>)
    static okser::result<Enum> deserialize(Context &context) {
        auto result = uint<Bytes, Endianness>::template deserialize<Underlying>(context);

        return result.transform([](const auto u) { return static_cast<Enum>(u); });
    }
};

/**
 * A null-terminated string
 * @ingroup simple_types
 * @tparam Terminator Allows overriding the default terminator character, which is `\0`
 */
template<uint8_t Terminator = 0>
struct terminated_string : public internal::type {
    using DefaultType = std::string;

    template<std::ranges::input_range S, OutputContext Context>
    static empty_result serialize(Context &&out, const S &string) {
        for (const auto &c: string) {
            if (auto result = out->add(c); !result) {
                return result;
            }
        }

        auto result = out->add('\0');
        return result;
    }

    template<std::ranges::range S, InputContext Context>
    static okser::result<S> deserialize(Context &context) {
        S output = S();
        auto it = okser::internal::get_fixed_or_dynamic_iterator<S>(output);

        bool ran_out_of_output = false;

        while (true) {
            auto c = context->get();

            if (!c) {
                // Error returned by input, return it and stop processing
                return std::unexpected(c.error());
            }

            if (*c == Terminator) {
                // Input string ran out
                break;
            }

            if (!ran_out_of_output) {
                if (internal::is_range_end(output, it)) {
                    ran_out_of_output = true;
                } else {
                    *it = *c;
                    it++;
                }
            }
        }

        if (ran_out_of_output) {
            return std::unexpected(okser::error_type::not_enough_output_bytes);
        }

        return output;
    }
};

using null_string = terminated_string<>;

}
