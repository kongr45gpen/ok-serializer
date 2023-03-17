#include "ok-serializer/ok-serializer.hpp"

uint8_t deserialize_uint8(std::string_view str) {
    auto [number] = okser::deserialize<okser::bundle<okser::uint<1>>, okser::in::range<std::string_view>, uint8_t>({str});
    return number;
}

uint8_t deserialize_uint32_be(std::string_view str) {
    auto [number] = okser::deserialize<okser::bundle<okser::uint<4, okser::end::be>>, okser::in::range<std::string_view>, uint32_t>({str});
    return number;
}

uint8_t deserialize_uint32_le(std::string_view str) {
    auto [number] = okser::deserialize<okser::bundle<okser::uint<4, okser::end::le>>, okser::in::range<std::string_view>, uint32_t>({str});
    return number;
}