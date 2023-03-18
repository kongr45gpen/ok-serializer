#include "catch2/catch.hpp"
#include "ok-serializer/ok-serializer.hpp"

using Catch::Matchers::Equals;

TEST_CASE("uint decoding") {
    SECTION("uint8_t") {
        std::string str = "\x7C";
        auto number = okser::deserialize<okser::uint<1>>(str);

        static_assert(std::is_same_v<decltype(number), uint8_t>);

        CHECK(number == 124);
    }

    SECTION("uint16_t big-endian") {
        std::string str = "\x43\xB0";
        auto number = okser::deserialize<okser::uint<2, okser::end::be>>(str);

        static_assert(std::is_same_v<decltype(number), uint16_t>);

        CHECK(number == 17328);
    }

    SECTION("uint16_t little-endian") {
        std::string str = "\xB0\x43";
        auto number = okser::deserialize<okser::uint<2, okser::end::le>>(str);

        static_assert(std::is_same_v<decltype(number), uint16_t>);

        CHECK(number == 17328);
    }

    SECTION("uint24_t big-endian") {
        std::string str = "\x05\x7B\xB8";
        auto number = okser::deserialize<okser::uint<3, okser::end::be>>(str);

        static_assert(std::is_same_v<decltype(number), uint32_t>);

        CHECK(number == 359352);
    }

    SECTION("uint24_t little-endian") {
        std::string str = "\xB8\x7B\x05";
        auto number = okser::deserialize<okser::uint<3, okser::end::le>>(str);

        static_assert(std::is_same_v<decltype(number), uint32_t>);

        CHECK(number == 359352);
    }
}

TEST_CASE("bundle decoding") {
    SECTION("Explicit bundle specification, 1 element") {
        std::string str = "\xB8\x7B\x05";
        using Bundle = okser::bundle<okser::uint<1>>;

        auto tuple = okser::deserialize<Bundle, std::tuple<uint8_t>>(str);

        CHECK(std::get<0>(tuple) == 0xB8);
    }

    SECTION("Explicit bundle specification, multiple elements") {
        std::string str = "\xB8\x7B\x05";
        using Bundle = okser::bundle<okser::uint<1>, okser::uint<1>, okser::uint<1>>;

        auto tuple = okser::deserialize<Bundle, std::tuple<uint8_t, uint8_t, uint8_t>>(str);

        static_assert(std::is_same_v<decltype(tuple), std::tuple<uint8_t, uint8_t, uint8_t>>);

        CHECK(std::get<0>(tuple) == 0xB8);
        CHECK(std::get<1>(tuple) == 0x7B);
        CHECK(std::get<2>(tuple) == 0x05);
    }

    SECTION("Automatic bundle type deduction") {
        std::string str = "\xB8\x7B\x05";
        using Bundle = okser::bundle<okser::uint<1>, okser::uint<2>>;

        auto tuple = okser::deserialize<Bundle>(str);

        static_assert(std::is_same_v<decltype(tuple), std::tuple<uint8_t, uint16_t>>);

        CHECK(std::get<0>(tuple) == 0xB8);
        CHECK(std::get<1>(tuple) == 0x7B05);
    }
}

TEST_CASE("static-time deserialization") {
    SECTION("full deserialization") {
        constexpr std::string_view str("\xB0\x43");
        constexpr okser::in::range<std::string_view> in(str);
        using Bundle = okser::bundle<okser::uint<2, okser::end::le>>;

        constexpr std::pair<std::optional<uint8_t>, decltype(in)> output_tuple = in.get();
        CHECK(output_tuple.first.has_value());
        CHECK(output_tuple.first.value() == 0xB0);

        constexpr auto output = okser::deserialize<Bundle, std::tuple<uint16_t>>(in);
        CHECK(std::get<0>(output) == 17328);
    }

    SECTION("quick deserialization") {
        constexpr std::string_view str("\x43\xB0");

        constexpr auto output = okser::deserialize<okser::uint<2>>(str);
        CHECK(output == 17328);
    }
}