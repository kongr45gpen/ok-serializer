#pragma once

#include <experimental/reflect>
#include "../example/mirror.hpp"
#include "ok-serializer/ok-serializer.hpp"
#include <iostream>

namespace okser {

namespace internal {
    struct empty {};
}

struct configuration {
    okser::end endianness = okser::end::be;
};

namespace internal {
    template<class S>
    constexpr configuration get_configuration_from_struct(const S& input) {
        configuration config;

        auto mirrored_config = mirror(configuration);
        auto mirrored_struct = mirror(S);
        for_each(get_data_members(mirrored_struct), [&](auto member) {
            // const auto &value = get_value(member, object);
            auto name = get_name(member);
            
            for_each(get_data_members(mirrored_config), [&](auto member_config) {
                auto name_config = get_name(member_config);

                if (name_config == name) {
                    get_reference(member_config, config) = get_value(member, input);
                }
            });
        });

        return config;
    }
}

template<class T, configuration Config>
struct default_serializers;

template<std::unsigned_integral T, configuration Config>
struct default_serializers<T, Config> {
    using ser = okser::uint<sizeof(T), Config.endianness>;
    using deser = okser::uint<sizeof(T), Config.endianness>;
};

template<std::signed_integral T, configuration Config>
struct default_serializers<T, Config> {
    using ser = okser::sint<sizeof(T), Config.endianness>;
    using deser = okser::sint<sizeof(T), Config.endianness>;
};


template<class T, Output Out, auto config = internal::empty{}>
constexpr void serialize_struct(Out &&output, const T &object) {
    constexpr auto populated_config = internal::get_configuration_from_struct(config);

    auto mirrored_struct = mirror(T);
    for_each(get_data_members(mirrored_struct), [&](auto member) {
        std::cerr << "Member:" << get_name(member) << "\t Type: " << get_name(get_type(member)) << std::endl;

        const auto &value = get_value(member, object);
        using type = std::remove_cvref_t<decltype(value)>;

        using serializer = default_serializers<type, populated_config>::ser;

        serializer::serialize(output, value);
    });
}

template<class T, auto config = internal::empty{}>
constexpr std::string serialize_struct_to_string(const T &object) {
    std::string out;
    serialize_struct<T, out::stdstring, config>({out}, object);

    return out;
}

template<class T, class In, auto config = internal::empty{}>
constexpr T deserialize_struct(In &&input) {
    constexpr auto populated_config = internal::get_configuration_from_struct(config);

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

        using deserializer = default_serializers<type, populated_config>::deser;

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
