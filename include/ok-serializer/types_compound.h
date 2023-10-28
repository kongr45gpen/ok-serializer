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

    template<OutputContext Context, typename... Values>
    requires (Serializer<Types>, ...)

    constexpr static empty_result serialize(Context &&output, Values... values) {
        std::tuple<serializable_value<Types, Values>...> typeValues{values...};

        std::apply(
                [&output](auto &&...v) { ((internal::serialize_one(output, v)), ...); },
                typeValues);

        // TODO: Actually parse errors occurred during serialization
        return {};
    }

    template<class Tuple, InputContext Context>
    requires (Deserializer<Types>, ...)
    constexpr static std::pair<okser::result<Tuple>, Context> deserialize(Context context) {
        using ValuesTuple = Tuple;

        std::optional<Context> in = context;

        // Loop through all elements of the tuple at compile time
        auto values = internal::apply([&in](const auto i) {
            // i cannot be defined as `constexpr`, so its value (i.e. the loop index)
            // is stored in a class type (std::integral_constant). Here we fetch the
            // loop index from this type, and store it in a constexpr variable.
            // This then allows us to throw it inside templates.
            constexpr auto Index = decltype(i)::value;
            using Serializer = std::tuple_element_t<Index, TypesTuple>;
            using Value = std::tuple_element_t<Index, ValuesTuple>;

            // If a previous field showed an error, do not parse this field
            if (in->error) {
                return okser::result<Value>(std::unexpected(*(in->error)));
            }

            auto [value, newIn] = Serializer::template deserialize<Value>(*in);

            // Forces using the copy constructor, instead of the copy assignment operator, which might not be usable
            // in constant environments.
            in.emplace(newIn);

            if (!value) {
                in->error = value.error();
            }

            return value;
        }, IndexSequence());

        if (!in->error) {
            ValuesTuple raw_values = std::apply([](auto &&...r) {
                return std::make_tuple((*r)...);
            }, values);

            return std::make_pair(raw_values, *in);
        } else {
            return std::make_pair(std::unexpected(*(in->error)), *in);
        }
    }
};

template<class T, int N> requires (N >= 1)
class redundant {
public:
    using DefaultType = typename T::DefaultType;

    template<OutputContext Context, typename Value>
    requires(Serializer<T>)
    constexpr static void serialize(Context &&output, const Value &value) {
        for (int i = 0; i < N; i++) {
            T::template serialize<Value, Context>(output, value);
        }
    }

    template<class Value, InputContext Context>
    requires(Deserializer<T>)
    constexpr static std::pair<okser::result<Value>, Context> deserialize(Context context) {
        auto [value, newContext] = T::template deserialize<Value>(context);

        bool ok = true;

        for (int i = 1; i < N; i++) {
            auto [nextValue, nextContext] = T::template deserialize<Value>(newContext);
            // TODO propagate errors?
            if (!nextValue || !value || nextValue.value() != value.value()) {
                ok = false;
            }
            newContext = nextContext;
        }

        if (ok) {
            return std::make_pair(value, newContext);
        } else {
            return std::make_pair(std::unexpected(okser::error_type::redundant_mismatch), newContext);
        }
    };
};

} // namespace okser