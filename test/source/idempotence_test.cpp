#include "catch2/catch.hpp"
#include "ok-serializer/ok-serializer.hpp"

using Catch::Matchers::Equals;
using namespace okser;

TEST_CASE("uint idempotence") {
    std::string str;
    okser::out::stdstring out{str};

    SECTION("uint8_t") {
        auto i = GENERATE(range(0, 255));

        serialize<bundle<okser::uint<1>>>(out, i);

        auto result = deserialize<okser::uint<1>>(str);

        CHECK(i == *result);
    }

    SECTION("sint64_t big endian") {
        uint64_t i = GENERATE(take(30, random(-1e12, 1e12)));

        serialize<bundle<okser::sint<8, end::be>>>(out, i);

        auto result = deserialize<okser::sint<8, end::be>>(str);

        CHECK(i == *result);
    }

    SECTION("sint64_t little endian") {
        uint64_t i = GENERATE(take(30, random(-1e12, 1e12)));

        serialize<bundle<okser::sint<8, end::le>>>(out, i);;

        auto result = deserialize<okser::sint<8, end::le>>(str);

        CHECK(i == *result);
    }
}

TEST_CASE("floatp idempotence") {
    SECTION("single-precision") {
        float i = GENERATE(take(100, random(-1e30, 1e30)));

        std::string str("");
        uint8_t *u8str = reinterpret_cast<uint8_t *>(str.data());
        okser::out::stdstring out{str};

        serialize<bundle<okser::floatp<4>>>(out, i);

        auto result = deserialize<okser::floatp<4>>(str);

        REQUIRE(result.has_value());
        CHECK(i == *result);
    }

}