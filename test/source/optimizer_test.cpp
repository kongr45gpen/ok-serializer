#include "ok-serializer/ok-serializer.hpp"

uint8_t deserialize_uint8(std::string_view str) {
    auto number = okser::deserialize<okser::uint<1>, uint8_t, okser::in::range<std::string_view>>({str});
    return number;
}

uint8_t deserialize_uint32_be(std::string_view str) {
    auto [number] = okser::deserialize<okser::bundle<okser::uint<4, okser::end::be>>, std::tuple<uint32_t>, okser::in::range<std::string_view>>(
            {str});
    return number;
}

uint8_t deserialize_uint32_le(std::string_view str) {
    auto [number] = okser::deserialize<okser::bundle<okser::uint<4, okser::end::le>>, std::tuple<uint32_t>, okser::in::range<std::string_view>>(
            {str});
    return number;
}