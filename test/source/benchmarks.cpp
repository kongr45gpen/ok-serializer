#include <catch2/catch_test_macros.hpp>
#include <catch2/benchmark/catch_benchmark.hpp>
#include "ok-serializer/ok-serializer.hpp"

TEST_CASE("Float encoding - with okser") {
    auto str = std::string_view("\xEF\xD2\x57\xFC");

    BENCHMARK("Simple float deserialize - le") {
                                                   return okser::deserialize<okser::floatp<4, okser::end::le>>(str);
                                               };

    BENCHMARK("Simple float deserialize - be") {
                                                   return okser::deserialize<okser::floatp<4, okser::end::be>>(str);
                                               };

    BENCHMARK("Simple float reinterpret_cast") {
                                                   return *reinterpret_cast<const float *>(str.data());
                                               };

    BENCHMARK("Literally nothing") {
                                       return 1e29;
                                   };
}