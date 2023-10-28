#include <iostream>

#include "ok-serializer/ok-serializer.hpp"

auto main() -> int {
    using namespace okser;

    std::string result;

    // A bundle represents a sequence of serializable types
    using bundle = bundle<sint<1>, okser::uint<2>>;

    // An output represents where you want to place the result of the serialization
    auto output = out::stdstring{result};

    serialize<bundle>(output, 104, 26913);

    // If you don't want to explicitly define an output, you can simple_serialize to an std::string
    std::cout << result << " " << serialize_to_string<sint<1>, okser::uint<2>>(104, 26913) << std::endl;
}
