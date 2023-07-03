#include <experimental/reflect>
#include "mirror.hpp"
#include "ok-serializer/ok-serializer.hpp"
#include "ok-serializer/reflection.h"
#include <concepts>

#include <iostream>

struct Structure {
    int8_t a;
    uint16_t b;
};


auto main() -> int {
    using namespace std::experimental;

    Structure s{104, 26913};

    using ss = reflexpr(Structure);
    std::cout << "Structure has " << reflect::get_size_v<reflect::get_data_members_t<ss>> << " data members"
              << std::endl;

    std::string result;
    okser::out::stdstring out(result);

    okser::serialize_struct(out, s);

    std::cout << "Result: " << result << std::endl;

    okser::deserialize_struct<Structure>(okser::in::range(result));
}

