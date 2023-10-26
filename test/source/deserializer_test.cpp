#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_string.hpp>
#include <catch2/matchers/catch_matchers_range_equals.hpp>
#include "ok-serializer/ok-serializer.hpp"

using Catch::Matchers::Equals;
using Catch::Matchers::RangeEquals;
using namespace std::string_literals;

template<typename T, typename U>
constexpr auto is_result = std::is_same_v<T, okser::result<U>>;

TEST_CASE("uint decoding") {
    SECTION("uint8_t") {
        std::string str = "\x7C";
        auto number = okser::deserialize<okser::uint<1>>(str);

        static_assert(is_result<decltype(number), uint8_t>);

        CHECK(number == 124);
    }

    SECTION("uint16_t big-endian") {
        std::string str = "\x43\xB0";
        auto number = okser::deserialize<okser::uint<2, okser::end::be>>(str);

        static_assert(is_result<decltype(number), uint16_t>);

        CHECK(number == 17328);
    }

    SECTION("uint16_t little-endian") {
        std::string str = "\xB0\x43";
        auto number = okser::deserialize<okser::uint<2, okser::end::le>>(str);

        static_assert(is_result<decltype(number), uint16_t>);

        CHECK(number == 17328);
    }

    SECTION("uint24_t big-endian") {
        std::string str = "\x05\x7B\xB8";
        auto number = okser::deserialize<okser::uint<3, okser::end::be>>(str);

        static_assert(is_result<decltype(number), uint32_t>);

        CHECK(number == 359352);
    }

    SECTION("uint24_t little-endian") {
        std::string str = "\xB8\x7B\x05";
        auto number = okser::deserialize<okser::uint<3, okser::end::le>>(str);

        static_assert(is_result<decltype(number), uint32_t>);

        CHECK(number == 359352);
    }
}

TEST_CASE("null-terminated string decoding") {
    std::string str = "Burgebrach\0"s;

    SECTION("null-terminated to dynamic string") {
        auto result = okser::deserialize<okser::null_string, std::string>(str);

        CHECK_THAT(*result, Equals("Burgebrach"));
    }

    SECTION("null-terminated to fixed string, equal size") {
        std::array<uint8_t, 10> expected_result = {'B', 'u', 'r', 'g', 'e', 'b', 'r', 'a', 'c', 'h'};

        auto result = okser::deserialize<okser::null_string, std::array<uint8_t, 10>>(str);

        CHECK_THAT(*result, RangeEquals(expected_result));
    }

    SECTION("null-terminated to larger fixed string") {
        std::array<uint8_t, 15> expected_result = {'B', 'u', 'r', 'g', 'e', 'b', 'r', 'a', 'c', 'h', 0, 0, 0, 0, 0};

        auto result = okser::deserialize<okser::null_string, std::array<uint8_t, 15>>(str);

        CHECK_THAT(*result, RangeEquals(expected_result));
    }

    SECTION("null-terminated to smaller fixed string") {
        auto result_small = okser::deserialize<okser::null_string, std::array<uint8_t, 9>>(str);
        auto result_smaller = okser::deserialize<okser::null_string, std::array<uint8_t, 8>>(str);

        REQUIRE_FALSE(result_small.has_value());
        REQUIRE_FALSE(result_smaller.has_value());

        CHECK(result_small.error().type == okser::error_type::not_enough_output_bytes);
        CHECK(result_smaller.error().type == okser::error_type::not_enough_output_bytes);
    }

    SECTION("null-terminated string without terminator") {
        auto error_str = "Burgebrach"s;
        auto result = okser::deserialize<okser::terminated_string<>, std::array<uint8_t, 15>>(error_str);

        REQUIRE_FALSE(result.has_value());
        CHECK(result.error().type == okser::error_type::not_enough_input_bytes);
    }

    SECTION("arbitrary-terminated to dynamic string") {
        auto result = okser::deserialize<okser::terminated_string<'b'>, std::string>(str);

        CHECK_THAT(*result, Equals("Burge"));
    }
}

TEST_CASE("bundle decoding") {
    SECTION("Explicit bundle specification, 1 element") {
        std::string str = "\xB8\x7B\x05";
        using Bundle = okser::bundle<okser::uint<1>>;

        auto tuple = okser::deserialize<Bundle, std::tuple<uint8_t>>(str);

        CHECK(std::get<0>(*tuple) == 0xB8);
    }

    SECTION("Explicit bundle specification, multiple elements") {
        std::string str = "\xB8\x7B\x05";
        using Bundle = okser::bundle<okser::uint<1>, okser::uint<1>, okser::uint<1>>;

        auto tuple = okser::deserialize<Bundle, std::tuple<uint8_t, uint8_t, uint8_t>>(str);

        static_assert(is_result<decltype(tuple), std::tuple<uint8_t, uint8_t, uint8_t>>);

        CHECK(std::get<0>(*tuple) == 0xB8);
        CHECK(std::get<1>(*tuple) == 0x7B);
        CHECK(std::get<2>(*tuple) == 0x05);
    }

    SECTION("Automatic bundle type deduction") {
        std::string str = "\xB8\x7B\x05";
        using Bundle = okser::bundle<okser::uint<1>, okser::uint<2>>;

        auto tuple = okser::deserialize<Bundle>(str);

        static_assert(is_result<decltype(tuple), std::tuple<uint8_t, uint16_t>>);

        CHECK(std::get<0>(*tuple) == 0xB8);
        CHECK(std::get<1>(*tuple) == 0x7B05);
    }
}

TEST_CASE("static-time deserialization") {
    SECTION("full deserialization") {
        constexpr std::string_view str("\xB0\x43");
        constexpr okser::in::range<std::string_view> in(str);
        using Bundle = okser::bundle<okser::uint<2, okser::end::le>>;

        constexpr auto output_tuple = in.get();
        CHECK(output_tuple.first.has_value());
        CHECK(output_tuple.first.value() == 0xB0);

        constexpr auto output = okser::deserialize<Bundle, std::tuple<uint16_t>>(in);
        CHECK(std::get<0>(*output) == 17328);
    }

    SECTION("quick deserialization") {
        constexpr std::string_view str("\x43\xB0");

        constexpr auto output = okser::deserialize<okser::uint<2>>(str);
        CHECK(*output == 17328);
    }
}

TEST_CASE("redundant decoding") {
    SECTION("Triple modular redundancy - correct") {
        std::string str = "\x42\x42\x42";

        auto number = okser::deserialize<okser::redundant<okser::uint<1>, 3>>(str);

        REQUIRE(number.has_value());
        CHECK(*number == 0x42);
    }

    SECTION("Triple modular redundancy - correct") {
        std::string str = "\x42\x41\x42";

        auto number = okser::deserialize<okser::redundant<okser::uint<1>, 3>>(str);

        REQUIRE_FALSE(number.has_value());
    }
}