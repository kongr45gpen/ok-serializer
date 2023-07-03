#pragma once

#include <experimental/reflect>
#include "../example/mirror.hpp"
#include "ok-serializer/ok-serializer.hpp"
#include <iostream>

namespace okser {

// TODO: Move these to a configuration class
auto constexpr default_endianness = okser::end::be;

template<class T>
struct default_serializers;

template<std::unsigned_integral T>
struct default_serializers<T> {
    using ser = okser::uint<sizeof(T), default_endianness>;
    using deser = okser::uint<sizeof(T), default_endianness>;
};

template<std::signed_integral T>
struct default_serializers<T> {
    using ser = okser::sint<sizeof(T), default_endianness>;
    using deser = okser::sint<sizeof(T), default_endianness>;
};


template<class T, Output Out>
constexpr void serialize_struct(Out &&output, const T &object) {
    auto mirrored_struct = mirror(T);

    for_each(get_data_members(mirrored_struct), [&](auto member) {
        std::cerr << "Member:" << get_name(member) << "\t Type: " << get_name(get_type(member)) << std::endl;

        const auto &value = get_value(member, object);
        using type = std::remove_cvref_t<decltype(value)>;

        using serializer = default_serializers<type>::ser;

        serializer::serialize(value, output);
    });
}

template<class T>
constexpr std::string serialize_struct_to_string(const T &object) {
    std::string out;
    serialize_struct(out::stdstring{out}, object);

    return out;
}

template<class T, class In>
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

        using deserializer = default_serializers<type>::deser;

        // TODO: Error handling
        auto deserialized_result = deserializer::template deserialize<type>(*context);
        reference = deserialized_result.first.value();
        context.emplace(deserialized_result.second);

        std::cout << "Member:" << get_name(member) << "\t Type: " << get_name(get_type(member))
                  << " \t Value: "
                  << get_value(member, result) << std::endl;
    });

    return result;
}

} // namespace okser