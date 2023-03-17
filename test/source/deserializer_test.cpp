#include "catch2/catch.hpp"
#include "ok-serializer/ok-serializer.hpp"

using Catch::Matchers::Equals;

TEST_CASE("uint decoding") {
  SECTION("uint8_t") {
    std::string str = "\x7C";
    auto [number] = okser::deserialize<okser::bundle<okser::uint<1>>, okser::in::range<>, uint8_t>({str});

    CHECK(number == 124);
  }

  SECTION("uint16_t big-endian") {
    std::string str = "\x43\xB0";
    auto [number] = okser::deserialize<okser::bundle<okser::uint<2, okser::end::be>>, okser::in::range<>, uint16_t>({str});

    CHECK(number == 17328);
  }

  SECTION("uint16_t little-endian") {
    std::string str = "\xB0\x43";
    auto [number] = okser::deserialize<okser::bundle<okser::uint<2, okser::end::le>>, okser::in::range<>, uint16_t>({str});

    CHECK(number == 17328);
  }
}

TEST_CASE("weird stuff") {
    constexpr std::string_view str("\xB0\x43");
    constexpr okser::in::range<std::string_view> in(str);
    using Bundle = okser::bundle<okser::uint<2, okser::end::le>>;

    constexpr std::pair<std::optional<uint8_t>, decltype(in)> output_tuple = in.get();
    CHECK(output_tuple.first.has_value());
    CHECK(output_tuple.first.value() == 0xB0);

    constexpr auto output = okser::deserialize<Bundle, decltype(in), uint16_t>(in);
    CHECK(std::get<0>(output) == 17328);
}