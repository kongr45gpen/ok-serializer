#include "catch2/catch.hpp"
#include "ok-serializer/ok-serializer.hpp"

using Catch::Matchers::Equals;

TEST_CASE("stdstring output") {
  SECTION("uint8_t") {
    std::string string;
    okser::out::stdstring out{string};

    out.add(uint8_t{0x70});
    out.add(uint8_t{0xAF});

    CHECK_THAT(string, Equals("\x70\xAF"));
  }
}

TEST_CASE("stdstring input") {
    SECTION("to uint8_t") {
        std::string string("\x70\xAF");
        okser::in::range in{string};

        CHECK(in.get() == 0x70);
        CHECK(in.get() == 0xAF);
        CHECK_FALSE(in.get());
    }

    SECTION("to uint8_t array") {
        std::string string("\x70\xAF");

        {
            okser::in::range in{string};
            auto success_array = in.get<2>();

            CHECK(success_array->at(0) == 0x70);
            CHECK(success_array->at(1) == 0xAF);
        }

        {
            okser::in::range in{string};
            auto fail_array = in.get<3>();

            CHECK_FALSE(fail_array);
        }

        {
            okser::in::range in{string};
            auto small_array = in.get<1>();

            CHECK(small_array->at(0) == 0x70);
        }
    }

    SECTION("to stdstring") {
        std::string string("\x70\xAF");

        {
            okser::in::range in{string};
            auto success_string = in.get<std::string>(2);

            CHECK(success_string);
            CHECK_THAT(*success_string, Equals(string));
        }
    }
}