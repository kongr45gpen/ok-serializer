#include "ok-serializer/ok-serializer.hpp"

using namespace std::string_literals;

static_assert(okser::Serializer<okser::uint<1>>);
static_assert(okser::serialize_to_string<okser::uint<1>>(15) == "\x0F");
//static_assert(!okser::Serializer<std::array<uint8_t, 3>>);

//static_assert(okser::Deserializer<okser::uint<1>>);
//static_assert(!okser::Deserializer<std::array<uint8_t, 3>>);
static_assert(okser::deserialize<okser::uint<1>>("\x0A"s).value() == 10);

//static_assert(okser::Deserializer<okser::bundle<okser::uint<1>, okser::floatp<>>>);