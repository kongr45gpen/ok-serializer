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

        if (output.error) {
            return std::unexpected(output.error.value());
        }

        return {};
    }

    template<class Tuple, InputContext Context>
    requires (Deserializer<Types>, ...)

    constexpr static okser::result<Tuple> deserialize(Context &context) {
        using ValuesTuple = Tuple;

        // Loop through all elements of the tuple at compile time
        auto values = internal::apply([&context](const auto i) {
            // i cannot be defined as `constexpr`, so its value (i.e. the loop index)
            // is stored in a class type (std::integral_constant). Here we fetch the
            // loop index from this type, and store it in a constexpr variable.
            // This then allows us to throw it inside templates.
            constexpr auto Index = decltype(i)::value;
            using Serializer = std::tuple_element_t<Index, TypesTuple>;
            using Value = std::tuple_element_t<Index, ValuesTuple>;

            // If a previous field showed an error, do not parse this field
            if (context.error) {
                return okser::result<Value>(std::unexpected(*(context.error)));
            }

            auto value = Serializer::template deserialize<Value>(context);

            if (!value) {
                context.error = value.error();
            }

            return value;
        }, IndexSequence());

        if (!context.error) {
            ValuesTuple raw_values = std::apply([](auto &&...r) {
                return std::make_tuple((*r)...);
            }, values);

            return raw_values;
        } else {
            return std::unexpected(*(context.error));
        }
    }
};

template<typename T, typename Size = okser::uint<1>>
class length_prefixed_vector {
public:
    using DefaultType = std::vector<typename T::DefaultType>;

    template<OutputContext Context, std::ranges::range Range>
    requires(Serializer<T>)
    constexpr static empty_result serialize(Context &&output, const Range &range) {
        auto size = std::ranges::size(range);

        if (size > std::numeric_limits<typename Size::DefaultType>::max()) {
            return std::unexpected(okser::error_type::overflow);
        }

        if (auto result = Size::serialize(output, size); !result) {
            return result;
        }

        for (const auto &r: range) {
            if (auto result = T::serialize(output, r); !result) {
                return result;
            }
        }

        return {};
    }

    template<std::ranges::range Range, InputContext Context>
    requires Deserializer<Size>
    constexpr static okser::result<Range> deserialize(Context &context) {
        using ValueType = typename Range::value_type;

        auto size = Size::template deserialize<size_t>(context);

        if (!size) {
            return std::unexpected(size.error());
        }

        if (size == std::numeric_limits<size_t>::max()) {
            return std::unexpected(okser::error_type::overflow);
        }

        // TODO: It might be possible to preallocate the output
        Range output = Range();
        auto it = okser::internal::get_fixed_or_dynamic_iterator<Range>(output);

        if constexpr (!internal::dynamic_range<Range>) {
            if (size.value() > std::ranges::size(output)) {
                return std::unexpected(okser::error_type::not_enough_output_bytes);
            }
        }

        for (size_t i = 0; i < *size; i++) {
            auto v = T::template deserialize<ValueType>(context);

            if (!v) {
                // Error returned by input, return it and stop processing
                return std::unexpected(v.error());
            }

            *it = *v;
            it++;
        }

        return output;
    }
};

/**
 * A string prefixed by its number of bytes
 * @ingroup simple_types
 * @tparam Size a Serializer or Deserializer for the field representing the size of the string
 */
template<class Size = okser::uint<1>>
struct pascal_string : public internal::type {
    using DefaultType = std::string;

    template<std::ranges::input_range S, OutputContext Context>
    requires Serializer<Size>
    constexpr static empty_result serialize(Context &&out, const S &string) {
        return length_prefixed_vector<charp, Size>::serialize(out, string);
    }

    template<std::ranges::range S = DefaultType, InputContext Context>
    requires Deserializer<Size>
    constexpr static okser::result<S> deserialize(Context &context) {
        return length_prefixed_vector<charp, Size>::template deserialize<S>(context);
    }
};

/**
 * A fixed-size array of identical values.
 *
 * Serialization just repeats the same value N times.
 *
 * Deserialization fails if any of the values does not match.
 */
template<class T, int N> requires (N >= 1)
class redundant {
public:
    using DefaultType = typename T::DefaultType;

    template<OutputContext Context, typename Value>
    requires(Serializer<T>)
    constexpr static empty_result serialize(Context &&output, const Value &value) {
        for (int i = 0; i < N; i++) {
            auto result = T::template serialize<Value, Context>(output, value);

            if (!result) {
                return result;
            }
        }

        return {};
    }

    template<class Value, InputContext Context>
    requires(Deserializer<T>)
    constexpr static okser::result<Value> deserialize(Context &context) {
        auto value = T::template deserialize<Value>(context);

        if (!value) return value;

        bool ok = true;
        for (int i = 1; i < N; i++) {
            auto nextValue = T::template deserialize<Value>(context);

            if (!nextValue) return nextValue;

            if (*nextValue != *value || !ok) {
                ok = false;
                // We still need to continue de-serialising values, so that the adequate amount of bytes is skipped
            }
        }

        if (ok) {
            return value;
        } else {
            return std::unexpected(okser::error_type::redundant_mismatch);
        }
    };
};

} // namespace okser