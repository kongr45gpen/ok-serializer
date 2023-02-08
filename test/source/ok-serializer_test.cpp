#include "ok-serializer/ok-serializer.hpp"

auto main() -> int
{
  auto const result = name();

  return result == "ok-serializer" ? 0 : 1;
}
