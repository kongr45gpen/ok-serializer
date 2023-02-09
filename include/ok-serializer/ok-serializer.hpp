#pragma once

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
        template<class Pair, class Output>
        constexpr void serialize_one(Pair p, Output&& o) {
            Pair::SerializerType::serialize(p.value, o);
        }
    }

    template<class... Types>
    class bundle {
    public:
        template<class Output, typename... Values>
        constexpr static void serialize(Output&& output, Values... values) {
            std::tuple<mypair<Types, Values>...> typeValues{values...};

            std::apply([&output](auto &&... v) {
                ((internal::serialize_one(v, output)), ...);
            }, typeValues);
        }
    };

    template<class Bundle, class Output, typename... Values>
    constexpr void serialize(Output&& output, Values... values) {
        return Bundle::serialize(output, values...);
    }

}
