#pragma once

#include <bit>
#include <cstdint>
#include "concepts.h"
#include "utils_early.h"
#include "context.h"

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
 * @tparam Bytes The number of bytes in the binary representation of the integer
 * @tparam Endianness
 */
template<int Bytes, end Endianness = end::be> requires (Bytes > 0 && Bytes <= 8)
struct uint : public internal::type {
    using DefaultType = okser::internal::uint_bytes_to_type<Bytes>;

    template<typename V, Output Out>
    constexpr static void serialize(const V &v, Out &&o) {
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

    template<typename V, InputContext Context>
    constexpr static std::pair<okser::result<V>, Context> deserialize(Context context) {
        auto [result, input] = context.input.template get<Bytes>();
        context.input = input;

        if (!result) {
            return {0, context};
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

        return {value, context};
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
    using DefaultType = std::make_signed_t<okser::internal::uint_bytes_to_type<Bytes>>;

    template<typename V, Output Out>
    static void serialize(const V &v, Out &&o) {
        using Unsigned = std::make_unsigned_t<V>;
        Unsigned u = std::bit_cast<Unsigned>(v);
        uint<Bytes, Endianness>::serialize(u, o);
    }

    template<typename V, InputContext Context>
    static std::pair<okser::result<V>, Context> deserialize(Context context) {
        using Unsigned = std::make_unsigned_t<V>;
        okser::result<Unsigned> u;

        std::tie(u, context) = uint<Bytes, Endianness>::template deserialize<Unsigned>(context);
        u.transform(std::bit_cast<V, Unsigned>);

        return {u, context};
    }
};

/**
 * IEEE 754 floating point number
 * @tparam Bytes The number of bytes in the binary representation of the number,
 *               either 4 (single-precision, equivalent to C's `float`), or 8
 *               (double-precision, equivalent to C's `double`)
 * @tparam Endianness
 */
template<int Bytes = 4, end Endianness = end::be> requires (Bytes == 4 || Bytes == 8)
struct floatp : public internal::type {
    using DefaultType = std::conditional_t<Bytes == 4, float, double>;

    template<typename V, Output Out>
    requires(std::is_floating_point_v<V>)
    static void serialize(const V &v, Out &&o) {
        using Unsigned = std::conditional_t<Bytes == 4, uint32_t, uint64_t>;

        Unsigned u = std::bit_cast<Unsigned>(static_cast<DefaultType>(v));

        uint<Bytes, Endianness>::serialize(u, o);
    }

    template<typename V = DefaultType, InputContext Context>
    requires(std::is_floating_point_v<V>)
    static constexpr auto deserialize(Context context) {
        using Unsigned = std::conditional_t<Bytes == 4, uint32_t, uint64_t>;

        okser::result<Unsigned> u;
        std::tie(u, context) = uint<Bytes, Endianness>::template deserialize<Unsigned>(context);

        return std::make_pair(u.transform(std::bit_cast<DefaultType, Unsigned>), context);
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
template<typename Enum, int Bytes = sizeof(std::underlying_type<Enum>), end Endianness = end::be> requires (Bytes > 0 &&
                                                                                                            Bytes <=
                                                                                                            8 &&
                                                                                                            std::is_enum_v<Enum>)
struct enumv : public internal::type {
protected:
    using Underlying = std::underlying_type_t<Enum>;
public:
    using DefaultType = Enum;

    template<Output Out>
    static void serialize(const Enum &e, Out &&o) {
        uint<Bytes, Endianness>::serialize(static_cast<Underlying>(e), o);
    }

    template<typename E = Enum, InputContext Context>
    requires(std::is_same_v<E, Enum>)
    static std::pair<okser::result<Enum>, Context> deserialize(Context context) {
        auto [result, new_context] = uint<Bytes, Endianness>::template deserialize<Underlying>(context);

        return {result.transform([](const auto u) { return static_cast<Enum>(u); }), new_context};
    }
};


struct null_string : public internal::type {
    using DefaultType = std::string;

    template<std::ranges::input_range S, Output Out>
    static void serialize(const S &string, Out &&o) {
        for (const auto &c: string) {
            o.add(c);
        }
        o.add('\0');
    }

    template<std::ranges::range S, InputContext Context>
    static std::pair<okser::result<S>, Context> deserialize(Context context) {
        S output = S();
        auto it = okser::internal::get_fixed_or_dynamic_iterator<S>(output);

        bool ran_out_of_output = false;

        while (true) {
            auto [c, input] = context.input.get();
            context.input = input;

            if (!c) {
                // Error returned by input, return it and stop processing
                return {std::unexpected(c.error()), context};
            }

            if (*c == '\0') {
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
            return {std::unexpected(okser::error_type::not_enough_output_bytes), context};
        }

        return {output, context};
    }
};

}
