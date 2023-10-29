#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_string.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>
#include <catch2/generators/catch_generators_all.hpp>
#include "ok-serializer/ok-serializer.hpp"

using Catch::Matchers::Equals;
using Catch::Matchers::WithinRel;
using namespace okser;

TEST_CASE("uint idempotence") {
    SECTION("uint8_t") {
        auto i = GENERATE(range(0, 255));

        auto str = serialize_to_string<okser::uint<1>>(i);

        auto result = deserialize<okser::uint<1>>(str);

        CHECK(i == *result);
    }

    SECTION("sint64_t big endian") {
        uint64_t i = GENERATE(take(30, random(-1e12, 1e12)));

        auto str = serialize_to_string<okser::sint<8, end::be>>(i);

        auto result = deserialize<okser::sint<8, end::be>>(str);

        CHECK(i == *result);
    }

    SECTION("sint64_t little endian") {
        uint64_t i = GENERATE(take(30, random(-1e12, 1e12)));

        auto str = serialize_to_string<okser::sint<8, end::le>>(i);;

        auto result = deserialize<okser::sint<8, end::le>>(str);

        CHECK(i == *result);
    }
}

TEST_CASE("floatp idempotence") {
    SECTION("single-precision") {
        float i = GENERATE(take(100, random(-1e30, 1e30)));

        auto str = serialize_to_string<okser::floatp<4>>(i);

        auto result = deserialize<okser::floatp<4>>(str);

        REQUIRE(result.has_value());
        CHECK_THAT(i, WithinRel(*result));
    }
}

TEST_CASE("varint idempotence") {
    SECTION("uint64_t") {
        auto i = GENERATE(take(1000, random < uint64_t > (0, std::numeric_limits<uint64_t>::max())));

        auto str = serialize_to_string<okser::varint>(i);
        auto result = deserialize<okser::varint>(str);

        CHECK(i == *result);
    }

    SECTION("uint16_t") {
        auto i = GENERATE(take(1000, random < uint16_t > (0, std::numeric_limits<uint16_t>::max())));

        auto str = serialize_to_string<okser::varint>(i);
        auto result = deserialize<okser::varint>(str);

        CHECK(i == *result);
    }

    SECTION("varint and signed_varint equivalence") {
        auto i = GENERATE(take(100, random < uint32_t > (0, std::numeric_limits<int32_t>::max())));

        auto str1 = serialize_to_string<okser::varint>(i);
        auto str2 = serialize_to_string<okser::signed_varint<>>(i);

        CHECK(str1 == str2);
    }
}