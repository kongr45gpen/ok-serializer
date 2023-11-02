#pragma once


#include <experimental/reflect>
#include "../example/mirror.hpp"
#include "ok-serializer/ok-serializer.hpp"
#include <iostream>

namespace okser {

namespace internal {
    struct empty {};
}

/**
 * Default values and options for serialisation and deserialization of a struct
 */
template<okser::end Endianness = okser::end::be>
struct configuration {
    /**
     * Default serializers for various types
     *
     * This struct contains type aliases: `ser` for the serializer and `deser` for the deserializer.
     * The template parameter T defines the type to be serialized.
     *
     * The purpose of this class is to automatically convert between C++ types and their (de)serializers.
     *
     * You can override the default serializers by template specialization on a child class.
     *
     * ## Example
     * @code
     * configuration::default_serializers<uint32_t>::ser // okser::uint<4>
     * configuration::default_serializers<uint64_t>::ser // okser::uint<8>
     * configuration::default_serializers<int64_t>::ser // okser::sint<8>
     * configuration::default_serializers<float>::ser // okser::floatp
     * configuration::default_serializers<MyEnumeration>::ser // okser::enumv<MyEnumeration>
     * @endcode
     *
     * @tparam T The original type to be serialized, or the intended result of the deserialization
     * @tparam Config A configuration struct
     */
    template<class T>
    struct default_serializers;

    /// @private
    template<std::unsigned_integral T>
    struct default_serializers<T> {
        using ser = okser::uint<sizeof(T), Endianness>;
        using deser = okser::uint<sizeof(T), Endianness>;
    };

    /// @private
    template<std::signed_integral T>
    struct default_serializers<T> {
        using ser = okser::sint<sizeof(T), Endianness>;
        using deser = okser::sint<sizeof(T), Endianness>;
    };
};

#if __cpp_reflection >= 201902L || DOXYGEN

template<class T, Output Out, auto config = configuration()>
constexpr void serialize_struct(Out &&output, const T &object) {
    auto mirrored_struct = mirror(T);
    for_each(get_data_members(mirrored_struct), [&](auto member) {
        const auto &value = get_value(member, object);
        using type = std::remove_cvref_t<decltype(value)>;

        using serializer = decltype(config)::template default_serializers<type>::ser;

        auto result = serialize<serializer>(std::forward<Out>(output), value);
    });
}

template<class T, auto config = configuration()>
constexpr std::string serialize_struct_to_string(const T &object) {
    std::string out;
    serialize_struct<T, out::dynamic<>, config>(out::dynamic(out), object);

    return out;
}

template<class T, class In, auto config = configuration()>
constexpr T deserialize_struct(In &&input) {
    // TODO: Find a more presentable and repeatable way to do this
    auto contained_input = internal::convert_input_to_okser(std::forward<In>(input));

    // TODO: Reuse functionality from bundle
    std::optional<input_context<decltype(contained_input)>> context;
    context.emplace(contained_input);

    auto mirrored_struct = mirror(T);

    T result; // TODO: There may be no default constructor

    for_each(get_data_members(mirrored_struct), [&](auto member) {
        auto &reference = get_reference(member, result);
        using type = std::remove_cvref_t<decltype(reference)>;

        using deserializer = decltype(config)::template default_serializers<type>::deser;

        // TODO: Error handling
        auto deserialized_result = deserializer::template deserialize<type>(*context);
        reference = deserialized_result.value();
    });

    return result;
}

#endif

} // namespace okser
