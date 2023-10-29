#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_string.hpp>
#include "ok-serializer/ok-serializer.hpp"

using Catch::Matchers::Equals;
using namespace std::string_literals;

TEST_CASE("uint encoding") {
    SECTION("uint8_t") {
        auto out = okser::serialize_to_string<okser::uint<1>>(124);
        REQUIRE(out[0] == 0x7C);
    }

    SECTION("uint16_t big-endian") {
        auto out = okser::serialize_to_string<okser::uint<2, okser::end::be>>(17328);
        CHECK_THAT(out, Equals("\x43\xB0"));
    }

    SECTION("uint16_t little-endian") {
        auto out = okser::serialize_to_string<okser::uint<2, okser::end::le>>(17328);
        CHECK_THAT(out, Equals("\xB0\x43"));
    }

    SECTION("uint24_t big-endian") {
        auto out = okser::serialize_to_string<okser::uint<3, okser::end::be>>(359352);
        CHECK_THAT(out, Equals("\x05\x7B\xB8"));
    }

    SECTION("uint24_t little-endian") {
        auto out = okser::serialize_to_string<okser::uint<3, okser::end::le>>(359352);
        CHECK_THAT(out, Equals("\xB8\x7B\x05"));
    }
}

TEST_CASE("sint encoding") {
    SECTION("sint8_t") {
        auto out = okser::serialize_to_string<okser::sint<1>>(-99);
        CHECK_THAT(out, Equals("\x9D"));
    }

    SECTION("sint16_t big-endian") {
        auto out = okser::serialize_to_string<okser::sint<2, okser::end::be>>(-31315);
        CHECK_THAT(out, Equals("\x85\xAD"));
    }

    SECTION("sint16_t little-endian") {
        auto out = okser::serialize_to_string<okser::sint<2, okser::end::le>>(-31315);
        CHECK_THAT(out, Equals("\xAD\x85"));
    }

    SECTION("sint24_t big-endian") {
        auto out = okser::serialize_to_string<okser::uint<3, okser::end::be>>(-7016456);
        CHECK_THAT(out, Equals("\x94\xEF\xF8"));
    }

    SECTION("sint24_t little-endian") {
        auto out = okser::serialize_to_string<okser::uint<3, okser::end::le>>(-7016456);
        CHECK_THAT(out, Equals("\xF8\xEF\x94"));
    }
}

TEST_CASE("float encoding") {
    SECTION("floatp big-endian") {
        auto out = okser::serialize_to_string<okser::floatp<4, okser::end::be>>(392.0853);
        CHECK_THAT(out, Equals("\x43\xC4\x0A\xEB"));
    }

    SECTION("floatp little-endian") {
        auto out = okser::serialize_to_string<okser::floatp<4, okser::end::le>>(392.0853);
        CHECK_THAT(out, Equals("\xEB\x0A\xC4\x43"));
    }

    SECTION("doublep big-endian") {
        auto out = okser::serialize_to_string<okser::floatp<8, okser::end::be>>(-1.9e158);
        CHECK_THAT(out, Equals("\xE0\xCB\xAD\x6C\x77\x40\x7F\x22"));
    }

    SECTION("doublep little-endian") {
        auto out = okser::serialize_to_string<okser::floatp<8, okser::end::le>>(-1.9e158);
        CHECK_THAT(out, Equals("\x22\x7F\x40\x77\x6C\xAD\xCB\xE0"));
    }
}

TEST_CASE("enum encoding") {
    enum class TestEnum : uint8_t {
        A = 0x01,
        B = 0x02,
        C = 0x03,
    };

    SECTION("8-bit enum to 8-bit value") {
        auto out = okser::serialize_to_string<okser::enumv<TestEnum>>(TestEnum::A);
        CHECK_THAT(out, Equals("\x01"));
    }

    SECTION("8-bit enum to 16-bit value") {
        auto out = okser::serialize_to_string<okser::enumv<TestEnum, 2>>(TestEnum::B);
        CHECK(out[0] == 0x00);
        CHECK(out[1] == 0x02);
    }
}

TEST_CASE("varint encoding") {
    SECTION("1-byte varint") {
        // protobuf.dev example
        auto out = okser::serialize_to_string<okser::varint>(1u);
        CHECK_THAT(out, Equals("\x01"));
    }

    SECTION("2-byte varint") {
        // protobuf.dev example
        auto out = okser::serialize_to_string<okser::varint>(150u);
        CHECK_THAT(out, Equals("\x96\x01"));
    }

    SECTION("10-byte varint") {
        // tested with python3 construct
        auto out = okser::serialize_to_string<okser::varint>(13183555200652522417U);
        CHECK_THAT(out, Equals("\xB1\xE7\x96\x9D\xB0\xB3\xD9\xFA\xB6\x01"));
    }

    SECTION("signed_varint") {
        // protobuf.dev example
        auto out = okser::serialize_to_string<okser::signed_varint<>>(int64_t{-2});
        CHECK_THAT(out, Equals("\xFE\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\x01"));

        out = okser::serialize_to_string<okser::signed_varint<4>>(-2);
        CHECK_THAT(out, Equals("\xFE\xFF\xFF\xFF\x0F"));
    }

    SECTION("zigzag varint") {
        auto serialization_of = [](int64_t i) {
            return okser::serialize_to_string<okser::zig_varint>(i);
        };

        CHECK_THAT(serialization_of(0), Equals("\x00"s));
        CHECK_THAT(serialization_of(-1), Equals("\x01"s));
        CHECK_THAT(serialization_of(1), Equals("\x02"s));
        CHECK_THAT(serialization_of(-2), Equals("\x03"s));
        CHECK_THAT(serialization_of(0x7fffffff), Equals("\xFE\xFF\xFF\xFF\x0F"s));
        CHECK_THAT(serialization_of(-(0x80000000L)), Equals("\xFF\xFF\xFF\xFF\x0F"s));
        CHECK_THAT(serialization_of(0x7fffffffffffffffL), Equals("\xFE\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\x01"s));
        CHECK_THAT(serialization_of(-(0x8000000000000000L)), Equals("\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\x01"s));
    }
}

TEST_CASE("null-terminated string encoding") {
    SECTION("fixed string serialisation") {
        std::array<uint8_t, 5> fixed_string = {'T', 'o', 'a', 's', 't'};

        auto out = okser::serialize_to_string<okser::null_string>(fixed_string);
        CHECK_THAT(out, Equals("Toast\x00"s));
    }

    SECTION("std::string serialisation") {
        std::string dynamic_string = "Abington";

        auto out = okser::serialize_to_string<okser::null_string>(dynamic_string);
        CHECK_THAT(out, Equals("Abington\x00"s));
    }

    SECTION("C string serialisation") {
        char c_string[] = "wind";

        auto out = okser::serialize_to_string<okser::null_string>(std::string_view(c_string));
        CHECK_THAT(out, Equals("wind\x00"s));

        out = okser::serialize_to_string<okser::null_string>(std::string_view(c_string, 2));
        CHECK_THAT(out, Equals("wi\x00"s));
    }
}

TEST_CASE("pascal string encoding") {
    std::string small_string = "small";
    std::string large_string = "The earth, the fire, the water and the majestic buffalo gathered around a table to discuss their laments. First of all, who would have thought that such a petty and mischievous achievement could overcome their joy? The Earth, a massive globe covered in lush forests and shimmering oceans, spoke first. \"I thought I'd been summoned for a grand geological council. You know, to discuss tectonic plate fashion trends or perhaps the latest in volcanic eruptions.\"";

    SECTION("small string") {
        auto out = okser::serialize_to_string<okser::pascal_string<okser::uint<1>>>(small_string);
        CHECK_THAT(out, Equals("\x05small"s));
    }

    SECTION("large string doesn't fit") {
        auto out = okser::serialize_to_string<okser::pascal_string<okser::uint<1>>>(large_string);
        CHECK_THAT(out, Equals(""s));
    }

    SECTION("large string") {
        auto out = okser::serialize_to_string<okser::pascal_string<okser::uint<2>>>(large_string);
        CHECK_THAT(out, Equals("\x01\xCB"s + large_string));
    }

    SECTION("large string with varint size") {
        auto out = okser::serialize_to_string<okser::pascal_string<okser::varint>>(large_string);
        CHECK_THAT(out, Equals("\xCB\x03"s + large_string));
    }
}

TEST_CASE("redundant encoding") {
    SECTION("Triple modular redundancy") {
        auto out = okser::serialize_to_string<okser::redundant<okser::uint<1>, 3>>(0x01);
        CHECK_THAT(out, Equals("\x01\x01\x01"));
    }
}