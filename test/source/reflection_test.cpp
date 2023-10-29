#if __cpp_reflection >= 201902L

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_string.hpp>
#include "ok-serializer/ok-serializer.hpp"
#include "ok-serializer/reflection.h"
#include "ok-serializer/types_json.h"

using Catch::Matchers::Equals;
using namespace std::string_literals;

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

struct my_configuration : public okser::configuration<okser::end::le> {
    template<class T>
    struct default_serializers {
        using ser = okser::uint<4>;
        using deser = okser::uint<4>;
    };
};

TEST_CASE("configuration") {
    SECTION("big endian") {
        constexpr auto config = okser::configuration<okser::end::be>();

        auto result = okser::serialize_struct_to_string<ThreeByteStructure, config>({119, -9153});

        CHECK(result[0] == 119);
        CHECK(result[1] == -36);
        CHECK(result[2] == 63);
    }

    SECTION("little endian") {
        constexpr auto config = okser::configuration<okser::end::le>();

        auto result = okser::serialize_struct_to_string<ThreeByteStructure, config>({119, -9153});

        CHECK(result[0] == 119);
        CHECK(result[1] == 63);
        CHECK(result[2] == -36);
    }

    SECTION("overriding serialisers") {
        constexpr auto config = my_configuration();

        auto result = okser::serialize_struct_to_string<ThreeByteStructure, config>({1, 1});
        CHECK_THAT(result, Equals("\x00\x00\x00\x01\x00\x00\x00\x01"s));

        auto original = okser::deserialize_struct<ThreeByteStructure, std::string, config>(
                "\x00\x00\x00\x01\x00\x00\x00\x01"s);
        CHECK(original.a == 1);
        CHECK(original.b == 1);
    }
}

TEST_CASE("json types") {
    SECTION("number") {
        auto result = okser::serialize_to_string<okser::json::number>(289);
        CHECK_THAT(result, Equals("289"s));

        result = okser::serialize_to_string<okser::json::number>(-1234567890);
        CHECK_THAT(result, Equals("-1234567890"s));
    }

    SECTION("string") {
        auto result = okser::serialize_to_string<okser::json::string>("toast"s);
        CHECK_THAT(result, Equals("\"toast\""s));

        result = okser::serialize_to_string<okser::json::string>("nikola\ntesla"s);
        CHECK_THAT(result, Equals(R"("nikola\ntesla")"s));

        result = okser::serialize_to_string<okser::json::string>("\"boop\\toast\":\t\r"s);
        CHECK_THAT(result, Equals(R"("\"boop\\toast\":\t\r")"s));

        result = okser::serialize_to_string<okser::json::string>("\x7F\x32\x00\x01\xFE"s);
        CHECK_THAT(result, Equals(R"("\x7F2\x00\x01\xFE")"s));
    }

    SECTION("array") {
        auto result = okser::serialize_to_string<okser::json::array<>>(std::vector<int>{1, 2, 3});
        CHECK_THAT(result, Equals("[1, 2, 3]"s));

        result = okser::serialize_to_string<okser::json::array<>>(std::vector<bool>{true, false, true, false});
        CHECK_THAT(result, Equals("[true, false, true, false]"s));

        result = okser::serialize_to_string<okser::json::array<>>(std::vector<std::string>{"toast"s, "is"s, "nice"s});
        CHECK_THAT(result, Equals(R"(["toast", "is", "nice"])"s));
    }
}

#endif
