#include "ok-serializer/ok-serializer.hpp"

using namespace okser;
using namespace std::string_literals;

// Concepts for Serializer
static_assert(Serializer<okser::uint<1>>);
static_assert(!Serializer<std::array<uint8_t, 3>>);

// Concepts for Deserializer
static_assert(Deserializer<okser::uint<1>>);
static_assert(Deserializer<bundle<okser::uint<1>, floatp<>>>);
//static_assert(!Deserializer<std::array<uint8_t, 3>>);

// Serialization at compile-time
static_assert(serialize_to_string<okser::uint<1>>(15) == "\x0F");
static_assert(serialize_to_string<okser::uint<1>, okser::uint<1>>(15, 32) == "\x0F\x20");

// Deserialization at compile-time
static_assert(deserialize<okser::uint<1>>("\x0A"s).value() == 10);
static_assert(std::get<1>(deserialize<okser::bundle<okser::uint<1>, okser::uint<1>>>("\x0F\x20"s).value()) == 32);
