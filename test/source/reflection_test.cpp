#if __cpp_reflection >= 201902L

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_string.hpp>
#include <catch2/matchers/catch_matchers_range_equals.hpp>
#include "ok-serializer/ok-serializer.hpp"
#include "ok-serializer/reflection.h"
#include "ok-serializer/types_json.h"

using Catch::Matchers::Equals;
using Catch::Matchers::RangeEquals;
using namespace std::string_literals;

struct ThreeByteStructure { 
    uint8_t a; 
    int16_t b; 
};

struct ComplexStructure {
    float number;
    std::string text;
    std::vector<int> numbers;
    ThreeByteStructure structure;
};

struct AlmostComplexStructure {
    float number;
    std::array<int, 3> numbers;
    ThreeByteStructure structure;
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

        result = okser::serialize_to_string<okser::json::array<>>(std::make_pair(1, false));
        CHECK_THAT(result, Equals(R"([1, false])"s));
    }

    SECTION("object") {
        ThreeByteStructure object{119, 86};
        auto result = okser::serialize_to_string<okser::json::object<>>(object);
        CHECK_THAT(result, Equals(R"({"a": 119, "b": 86})"s));

        ComplexStructure complex{1.4f, "toast", {1, 2, 3}, {119, 86}};
        result = okser::serialize_to_string<okser::json::object<>>(complex);
        CHECK_THAT(result,
                   Equals(R"({"number": 1.40, "text": "toast", "numbers": [1, 2, 3], "structure": {"a": 119, "b": 86}})"s));
    }

    SECTION("json at compile-time") {
        using namespace std::string_view_literals;

        constexpr auto number = okser::serialize_one_fixed<okser::json::number, uint16_t, std::array<uint8_t, 3>>(289);
        CHECK_THAT(number, RangeEquals("289"s));

        constexpr auto floating_point = okser::serialize_one_fixed<okser::json::number, float, std::array<uint8_t, 6>>(
                289.3);
        CHECK_THAT(floating_point, RangeEquals("289.30"s));

        constexpr auto string = okser::serialize_one_fixed<okser::json::string, std::string_view, std::array<uint8_t, 6>>(
                "hola"sv);
        CHECK_THAT(string, RangeEquals("\"hola\""s));

        constexpr auto array = okser::serialize_one_fixed<okser::json::array<>, std::vector<int>, std::array<uint8_t, 9>>(
                std::vector<int>{1, 2, 3});
        CHECK_THAT(array, RangeEquals("[1, 2, 3]"s));

        constexpr auto object = okser::serialize_one_fixed<okser::json::object<>, ThreeByteStructure, std::array<uint8_t, 19>>(
                {119, 86});
        CHECK_THAT(object, RangeEquals(R"({"a": 119, "b": 86})"s));

        constexpr AlmostComplexStructure complex_structure{1.4f, {1, 2, 3}, {119, 86}};
        constexpr auto complex = okser::serialize_one_fixed<okser::json::object<>, AlmostComplexStructure, std::array<uint8_t, 72>>(
                complex_structure);
        CHECK_THAT(complex,
                   RangeEquals(R"({"number": 1.40, "numbers": [1, 2, 3], "structure": {"a": 119, "b": 86}})"s));
    }
}

#endif
