#pragma once

#include "io.h"
#include "types_simple.h"
#include "utils_early.h"

namespace okser {

/**
 * A container for a serializable value, which includes static information about the serializer used for this value.
 * @tparam S The serializer type
 * @tparam V The value type
 */
template<Serializer S, typename V>
struct serializable_value {
    using SerializerType = S;
    using ValueType = V;
    ValueType value;

    constexpr serializable_value(const ValueType &v) : value(v) {}
};


/**
 * An array of types to be serialized. Bundle objects describe the binary format of the serialized output,
 * and are a required input of the serializer.
 *
 * ## Example
 * \code
 * using MyBundle = okser::bundle<okser::uint<4>, okser::sint<2>, okser::floatp>;
 * \endcode
 */
template<class... Types>
class bundle {
private:
    using IndexSequence = std::make_index_sequence<sizeof...(Types)>;
public:
    using TypesTuple = std::tuple<Types...>;
    using DefaultType = std::tuple<typename Types::DefaultType...>;

    template<Output Out, typename... Values>
    requires (Serializer<Types>, ...)
    constexpr static void serialize(Out &&output, Values... values) {
        std::tuple<serializable_value<Types, Values>...> typeValues{values...};

        std::apply(
                [&output](auto &&...v) { ((internal::serialize_one(v, output)), ...); },
                typeValues);
    }

    template<class Tuple, Input In>
    requires (Deserializer<Types>, ...)
    constexpr static std::pair<Tuple, In> deserialize(In input) {
        using ValuesTuple = Tuple;

        std::optional<In> in = input;

        // Loop through all elements of the tuple at compile time
        auto values = internal::apply([&in](const auto i) {
            // i cannot be defined as `constexpr`, so its value (i.e. the loop index)
            // is stored in a class type (std::integral_constant). Here we fetch the
            // loop index from this type, and store it in a constexpr variable.
            // This then allows us to throw it inside templates.
            constexpr auto Index = decltype(i)::value;
            using Serializer = std::tuple_element_t<Index, TypesTuple>;
            using Value = std::tuple_element_t<Index, ValuesTuple>;

            auto [value, newIn] = Serializer::template deserialize<Value>(*in);

            // Forces using the copy constructor, instead of the copy assignment operator, which might not be usable
            // in constant environments.
            in.emplace(newIn);

            return value;
        }, IndexSequence());

        return std::pair(values, *in);
    }
};

} // namespace okser