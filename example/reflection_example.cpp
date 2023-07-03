#include <experimental/reflect>
#include "mirror.hpp"
#include "ok-serializer/ok-serializer.hpp"
#include <concepts>

#include <iostream>

namespace okser {
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

}

struct Structure {
    int8_t a;
    uint16_t b;
    uint32_t c;
};


auto main() -> int {
    using namespace std::experimental;

    Structure s{104, 26913, 59460234};

    using ss = reflexpr(Structure);
    std::cout << "Structure has " << reflect::get_size_v<reflect::get_data_members_t<ss>> << " data members"
              << std::endl;

    std::string result;
    okser::out::stdstring out(result);

    for_each(get_data_members(mirror(Structure)), [&](auto member) {
        std::cout << "Member:" << get_name(member) << "\t Type: " << get_name(get_type(member)) << std::endl;

        const auto &value = get_value(member, s);
        using type = std::remove_cvref_t<decltype(value)>;

        using serializer = okser::default_serializers<type>::ser;

        serializer::serialize(get_value(member, s), out);
    });

    std::cout << "Result: " << result << std::endl;
}

