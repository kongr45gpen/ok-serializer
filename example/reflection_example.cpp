#include <experimental/reflect>
#include "mirror.hpp"
#include <iostream>

// To compile: clang++ -freflection-ts -freflection-ext -stdlib=libc++ -g -std=c++2b -o reflection_example reflection_example.cpp -v
// To run:     LD_LIBRARY_PATH=/path/to/llvm-project/build/lib/x86_64-unknown-linux-gnu ./reflection_example

struct Structure {
    int8_t a;
    uint16_t b;
};


auto main() -> int {
    using namespace std::experimental;

    Structure s{104, 26913};

    using ss = reflexpr(Structure);
    std::cout << "Structure has " << reflect::get_size_v < reflect::get_data_members_t <
    ss >> << " data members" << std::endl;

    std::string result;

    for_each(get_data_members(mirror(Structure)), [&](auto member) {
        std::cout << "Member:" << get_name(member) << "\t Type: " << get_name(get_type(member)) << std::endl;

        auto type_name = get_name(get_type(member));

        if (type_name == "signed char") {
            result.append(reinterpret_cast<const char *>(&(get_value(member, s))), sizeof(s.a));
        } else if (type_name == "unsigned short") {
            result.append(reinterpret_cast<const char *>(&(get_value(member, s))), sizeof(s.b));
        } else if (true) {
            std::cerr << "Unknown type " << type_name << std::endl;
        }
    });

    std::cout << "Result: " << result << std::endl;
}

