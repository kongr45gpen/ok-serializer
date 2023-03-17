#include "catch2/catch.hpp"
#include "ok-serializer/ok-serializer.hpp"

using Catch::Matchers::Equals;

TEST_CASE("uint decoding") {
  SECTION("uint8_t") {
    std::string str = "\x7C";
    auto [number] = okser::deserialize<okser::bundle<okser::uint<1>>, okser::in::range<>, uint8_t>({str});

    REQUIRE(number == 124);
  }

  SECTION("uint16_t big-endian") {
    std::string str = "\x43\xB0";
    auto [number] = okser::deserialize<okser::bundle<okser::uint<2, okser::end::be>>, okser::in::range<>, uint16_t>({str});

    REQUIRE(number == 17328);
  }

  SECTION("uint16_t little-endian") {
    std::string str = "\xB0\x43";
    auto [number] = okser::deserialize<okser::bundle<okser::uint<2, okser::end::le>>, okser::in::range<>, uint16_t>({str});

    REQUIRE(number == 17328);
  }
}