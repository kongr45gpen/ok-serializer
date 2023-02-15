#include <iostream>
#include <experimental/reflect>
#include <string>


#include "ok-serializer/ok-serializer.hpp"

struct Structure {
    uint8_t a;
    uint16_t b;
    int32_t c;
    float d;
};

namespace reflect = std::experimental::reflect;

template<reflect::Typed... Ms>
using tuple_from_seq_t = std::tuple<reflect::get_reflected_type_t<
        reflect::get_type_t<Ms>>...>;

template<reflect::Typed... Ms>
using simple_tuple_from_seq_t = std::tuple<Ms...>;

template<reflect::Record T>
using collect_tuple = reflect::unpack_sequence_t<
        simple_tuple_from_seq_t,
        reflect::get_data_members_t<T>>;

template <typename Tp>
constexpr std::string_view nameof() {
    using TpInfo = reflexpr(Tp);
    using aliased_Info = std::experimental::reflect::get_aliased_t<TpInfo>;
    return std::experimental::reflect::get_display_name_v<aliased_Info>;
}

template <typename T>
constexpr std::string wat() {
    using namespace std::string_literals;

//    using type = std::experimental::reflect::get_reflected_type_t<T>;
//    using scope = std::experimental::reflect::is_public<T>;
//    return std::to_string(std::experimental::reflect::is_public_v<std::remove_reference_t<T>>);
//    return std::to_string(std::experimental::reflect::is_public_v<reflexpr(Structure::a)>);
    return std::experimental::reflect::get_display_name_v<std::remove_reference_t<T>> + " "s
        + std::experimental::reflect::get_display_name_v<std::experimental::reflect::get_type_t<std::remove_reference_t<T>>> + " "s
        + std::experimental::reflect::get_name_v<std::remove_reference_t<T>>;
    return __PRETTY_FUNCTION__;
};

auto main() -> int {
    using namespace std::experimental::reflect;

    using members = get_public_data_members_t<reflexpr(Structure)>;
    auto size = get_size_v<members>;

    std::cout << nameof<members>() << std::endl;
    std::cout << std::experimental::reflect::get_display_name_v<reflexpr(Structure)> << std::endl;

    using members_tuple = collect_tuple<reflexpr(Structure)>;

    std::apply([](auto&&... args) {
        ((std::cout << wat<decltype(args)>() << '\n'), ...);
        }, members_tuple());

//    for(const auto& member : members_tuple()) {
//        std::cout << std::experimental::reflect::get_display_name_v<member> << std::endl;
//    }
}
