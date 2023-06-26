#include <iostream>

#include "ok-serializer/ok-serializer.hpp"

auto main() -> int {
    using namespace okser;

    std::string result;

    // A bundle represents a sequence of serializable types
    using bundle = bundle<sint<1>, okser::uint<2>>;

    // An output represents where you want to place the result of the serialization
    auto output = out::stdstring{result};

    serialize<bundle>(output, -20, 515);

    // If you don't want to explicitly define an output, you can simple_serialize to an std::string
    std::cout << std::hex << result << " " << simple_serialize<sint<1>, okser::uint<2>>(-20, 515) << std::endl;

    if (std::FILE *stream{std::fopen("test.bin", "w")}) {
        std::fwrite(result.data(), 1, result.size(), stream);
        std::fclose(stream);
    }
}
