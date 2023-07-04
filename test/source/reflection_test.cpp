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

TEST_CASE("configuration") {
    SECTION("big endian") {
        constexpr okser::configuration config { .endianness = okser::end::be };

        auto result = okser::serialize_struct_to_string<ThreeByteStructure, config>({119, -9153});

        CHECK(result[0] == 119);
        CHECK(result[1] == -36);
        CHECK(result[2] == 63);
    }

    SECTION("little endian") {
        constexpr okser::configuration config { .endianness = okser::end::le };

        auto result = okser::serialize_struct_to_string<ThreeByteStructure, config>({119, -9153});

        CHECK(result[0] == 119);
        CHECK(result[1] == 63);
        CHECK(result[2] == -36);
    }
}
