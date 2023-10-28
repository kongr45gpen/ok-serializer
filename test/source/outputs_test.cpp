#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_string.hpp>
#include "ok-serializer/ok-serializer.hpp"

using Catch::Matchers::Equals;

TEST_CASE("stdstring output") {
    SECTION("uint8_t") {
        std::string string;
        okser::out::dynamic out(string);

        out.add(uint8_t{0x70});
    out.add(uint8_t{0xAF});

    CHECK_THAT(string, Equals("\x70\xAF"));
  }
}

TEST_CASE("fixed-size output") {
    SECTION("within range") {
        std::string string = "   ";

        auto result = okser::serialize<okser::uint<2>>(okser::out::fixed_container(string), 0x6869);

        CHECK(result);
        CHECK_THAT(string, Equals("hi "));
    }

    SECTION("outside range") {
        std::string string = "   ";

        auto result = okser::serialize<okser::uint<4>>(okser::out::fixed_container(string), 0x68697961);

        CHECK(!result);
        CHECK(result.error()() == okser::error_type::not_enough_output_bytes);
        CHECK_THAT(string, Equals("hiy"));
    }
}

TEST_CASE("stdstring input") {
    SECTION("to uint8_t") {
        std::string string("\x70\xAF");
        okser::in::range in{string};
        okser::result<uint8_t> value;

        std::tie(value, in) = in.get();
        CHECK(*value == 0x70);

        std::tie(value, in) = in.get();
        CHECK(*value == 0xAF);

        std::tie(value, in) = in.get();
        CHECK_FALSE(value);
    }

    SECTION("to uint8_t array") {
        std::string string("\x70\xAF");

        {
            okser::in::range in{string};
            auto [success_array, _] = in.get<2>();

            CHECK(success_array->at(0) == 0x70);
            CHECK(success_array->at(1) == 0xAF);
        }

        {
            okser::in::range in{string};
            auto [fail_array, _] = in.get<3>();

            CHECK_FALSE(fail_array);
        }

        {
            okser::in::range in{string};
            auto [small_array, _] = in.get<1>();

            CHECK(small_array->at(0) == 0x70);
        }
    }

    SECTION("to stdstring") {
        std::string string("\x70\xAF");

        {
            okser::in::range in{string};
            auto [success_string, _] = in.get<std::string>(2);

            CHECK(success_string);
            CHECK_THAT(*success_string, Equals(string));
        }
    }
}