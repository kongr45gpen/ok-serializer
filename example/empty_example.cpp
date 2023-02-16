#include <iostream>

#include "ok-serializer/ok-serializer.hpp"

auto main() -> int {
    using namespace okser;

    std::string result;

    using bundle = bundle<sint<1>, okser::uint<2>>;

    auto output = out::stdstring{result};

    serialize<bundle>(output, -20, 515);

    std::cout << result << " " << simple_serialize<sint<1>,okser::uint<2>>(-20,515) << std::endl;

    if (std::FILE *stream{std::fopen("test.bin", "w")}) {
        std::fwrite(result.data(), 1, result.size(), stream);
        std::fclose(stream);
    }
}
