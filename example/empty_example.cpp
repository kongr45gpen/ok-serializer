#include <iostream>

#include "ok-serializer/ok-serializer.hpp"

auto main() -> int {
  std::string result;

  using bundle = okser::bundle<okser::sint<1>, okser::uint<2>>;

  auto output = okser::out::stdstring{result};

  okser::serialize<bundle>(output, -20, 515);

  std::cout << result << std::endl;

  if (std::FILE * stream{std::fopen("test.bin", "w")}) {
    std::fwrite(result.data(), 1, result.size(), stream);
    std::fclose(stream);
  }
}
