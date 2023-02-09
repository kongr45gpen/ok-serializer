#pragma once

#include <string>
#include <tuple>

#include "impl_types.h"
#include "impl_outputs.h"

namespace okser {
    template<typename T1, typename T2>
    struct mypair {
        using SerializerType = T1;
        using ValueType = T2;
        T2 value;
    };

    namespace internal {
        template<class Pair>
        std::string serialize_one(Pair p) {
            return Pair::SerializerType::serialize(p.value);
        }
    }

    template<class... Types>
    class bundle {
    public:
        template<typename... Values>
        static std::string serialize(Values... values) {
            std::string result = "";

            std::tuple<mypair<Types, Values>...> typeValues{values...};

            std::apply([&result](auto &&... v) {
//                ((result += std::to_string(v.value)), ...);
                ((result += internal::serialize_one(v)), ...);
            }, typeValues);

            return result;
        }
    };

    template<class Bundle, typename... Values>
    std::string serialize(Values... values) {
        return Bundle::serialize(values...);
    }

}
