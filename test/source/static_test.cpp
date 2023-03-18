#include "ok-serializer/ok-serializer.hpp"

static_assert(okser::Serializer<okser::uint<1>>);
static_assert(!okser::Serializer<std::array<uint8_t, 3>>);

static_assert(okser::Deserializer<okser::uint<1>>);
static_assert(!okser::Deserializer<std::array<uint8_t, 3>>);

static_assert(okser::Deserializer<okser::bundle<okser::uint<1>, okser::floatp<>>>);