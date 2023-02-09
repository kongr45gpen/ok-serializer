#include "catch2/catch.hpp"
#include "ok-serializer/ok-serializer.hpp"

using Catch::Matchers::Equals;

TEST_CASE( "uint encoding" ) {
    SECTION("uint8_t") {
        std::string out;
        okser::serialize<okser::bundle<okser::uint<1>>>(
                okser::out::stdstring{out},
                124
                );
        REQUIRE(out[0] == 0x7C);
    }

    SECTION("uint16_t big-endian") {
        std::string out;
        okser::serialize<okser::bundle<okser::uint<2, okser::end::be>>>(
                okser::out::stdstring{out},
                17328
            );
        CHECK_THAT(out, Equals("\x43\xB0"));
    }

    SECTION("uint16_t little-endian") {
        std::string out;
        okser::serialize<okser::bundle<okser::uint<2, okser::end::le>>>(
                okser::out::stdstring{out},
                17328
            );
        CHECK_THAT(out, Equals("\xB0\x43"));
    }
    SECTION("uint24_t big-endian") {
        std::string out;
        okser::serialize<okser::bundle<okser::uint<3, okser::end::be>>>(
                okser::out::stdstring{out},
                359352
            );
        CHECK_THAT(out, Equals("\x05\x7B\xB8"));
    }

    SECTION("uint24_t little-endian") {
        std::string out;
        okser::serialize<okser::bundle<okser::uint<3, okser::end::le>>>(
                okser::out::stdstring{out},
                359352
            );
        CHECK_THAT(out, Equals("\xB8\x7B\x05"));
    }
}

TEST_CASE( "sint encoding" ) {
    SECTION("sint8_t") {
        std::string out;
        okser::serialize<okser::bundle<okser::sint<1>>>(
                okser::out::stdstring{out},
                -99
                );
        CHECK_THAT(out, Equals("\x9D"));
    }

    SECTION("sint16_t big-endian") {
        std::string out;
        okser::serialize<okser::bundle<okser::sint<2, okser::end::be>>>(
                okser::out::stdstring{out},
                -31315
            );
        CHECK_THAT(out, Equals("\x85\xAD"));
    }

    SECTION("sint16_t little-endian") {
        std::string out;
        okser::serialize<okser::bundle<okser::sint<2, okser::end::le>>>(
                okser::out::stdstring{out},
                -31315
            );
        CHECK_THAT(out, Equals("\xAD\x85"));
    }
    SECTION("sint24_t big-endian") {
        std::string out;
        okser::serialize<okser::bundle<okser::uint<3, okser::end::be>>>(
                okser::out::stdstring{out},
                -7016456
            );
        CHECK_THAT(out, Equals("\x94\xEF\xF8"));
    }

    SECTION("sint24_t little-endian") {
        std::string out;
        okser::serialize<okser::bundle<okser::uint<3, okser::end::le>>>(
                okser::out::stdstring{out},
                -7016456
            );
        CHECK_THAT(out, Equals("\xF8\xEF\x94"));
    }
}
