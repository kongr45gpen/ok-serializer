#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_string.hpp>
#include "ok-serializer/ok-serializer.hpp"
#include "ok-serializer/reflection.h"

using Catch::Matchers::Equals;

struct ThreeByteStructure { 
    uint8_t a; 
    int16_t b; 
};

TEST_CASE("struct encoding") {
    SECTION("simple test") {
        auto result = okser::serialize_struct_to_string(ThreeByteStructure{119, -9153});

        CHECK(result[0] == 119);
        CHECK(result[1] == -36);
        CHECK(result[2] == 63);
    }
}

TEST_CASE("struct decoding") {
    SECTION("simple test") {
        std::array<signed char, 3> string = { 119, -36, 63 };

        auto result = okser::deserialize_struct<ThreeByteStructure>(string);

        static_assert(std::is_same_v<decltype(result), ThreeByteStructure>);

        CHECK(result.a == 119);
        CHECK(result.b == -9153);
    }
}

TEST_CASE("configuration") {
    SECTION("big endian") {
        constexpr struct { okser::end endianness = okser::end::be; } config;

        auto result = okser::serialize_struct_to_string<ThreeByteStructure, config>({119, -9153});

        CHECK(result[0] == 119);
        CHECK(result[1] == -36);
        CHECK(result[2] == 63);
    }

    SECTION("little endian") {
        constexpr struct { okser::end endianness = okser::end::le; } config;

        auto result = okser::serialize_struct_to_string<ThreeByteStructure, config>({119, -9153});

        CHECK(result[0] == 119);
        CHECK(result[1] == 63);
        CHECK(result[2] == -36);
    }
}
