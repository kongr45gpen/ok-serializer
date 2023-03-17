#include "catch2/catch.hpp"
#include "ok-serializer/ok-serializer.hpp"

using Catch::Matchers::Equals;

TEST_CASE("uint decoding") {
  SECTION("uint8_t") {
    std::string str = "\x7C";
    auto [number] = okser::deserialize<okser::bundle<okser::uint<1>>, okser::in::range<>, uint8_t>({str});

    REQUIRE(number == 124);
  }
}