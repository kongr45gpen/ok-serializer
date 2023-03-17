#pragma once

#include <tuple>

#include "impl_outputs.h"
#include "impl_types.h"
#include "impl_utils.h"

namespace okser {

/**
 * A container for a serializable value, which includes static information about the serializer used for this value.
 * @tparam S The serializer type
 * @tparam V The value type
 */
template <Serializer S, typename V> struct serializable_value {
  using SerializerType = S;
  using ValueType = V;
  ValueType value;

  constexpr serializable_value(const ValueType& v) : value(v) {}
};

template <Serializer S, typename V> struct deserializable_value {
  using SerializerType = S;
  using ValueType = V;

  // constexpr deserializable_value(const ValueType& v) : value(v) {}
};

// -DCMAKE_CXX_FLAGS:STRING="-isystem /home/kongr45gpen/llvm-project/build/include/c++/v1/ -isystem /home/kongr45gpen/llvm-project/build/include/x86_64-unknown-linux-gnu/c++/v1 -nostdinc++ -nostdlib++ -freflection-ts -L/home/kongr45gpen/llvm-project/build/lib/x86_64-unknown-linux-gnu -Wl,-rpath,/home/kongr45gpen/llvm-project/build/lib/x86_64-unknown-linux-gnu/ -lc++" -DBUILD_TESTING="ON"


/**
 * An array of types to be serialized. Bundle objects describe the binary format of the serialized output,
 * and are a required input of the serializer.
 *
 * ## Example
 * \code
 * using MyBundle = okser::bundle<okser::uint<4>, okser::sint<2>, okser::floatp>;
 * \endcode
 */
template <Serializer... Types> class bundle {
  using TypesTuple = std::tuple<Types...>;
  using IndexSequence = std::make_index_sequence<sizeof...(Types)>;
public:
  template <Output Out, typename... Values>
  constexpr static void serialize(Out &&output, Values... values) {
    std::tuple<serializable_value<Types, Values>...> typeValues{values...};

    std::apply(
        [&output](auto &&...v) { ((internal::serialize_one(v, output)), ...); },
        typeValues);
  }

  template <Input In, typename... Values>
  constexpr static std::tuple<Values...> deserialize(In input) {
    using ValuesTuple = std::tuple<Values...>;

    std::optional<In> in = input;

    // Loop through all elements of the tuple at compile time
    return internal::apply([&in] (const auto i) {
      // i cannot be defined as `constexpr`, so its value (i.e. the loop index)
      // is stored in a class type (std::integral_constant). Here we fetch the
      // loop index from this type, and store it in a constexpr variable.
      // This then allows us to throw it inside templates.
      constexpr auto Index = decltype(i)::value;
      using Serializer = std::tuple_element_t<Index, TypesTuple>;
      using Value = std::tuple_element_t<Index, ValuesTuple>;

      auto [value, newIn] = Serializer::template deserialize<Value>(*in);

      // this is stupid
      in.emplace(newIn);

      return value;
    }, IndexSequence());
  }
};

/**
 * Take some @p values, serialize them using a @p bundle, and append them to an @p output
 *
 * ## Example
 * \code
 * using bundle = okser::bundle<okser::sint<1>, okser::uint<2>>;
 * okser::serialize<bundle>(output, 100, 50000);
 * \endcode
 *
 * @tparam Bundle The bundle containing each element of the serialized structure.
 * @tparam Output The type of the output.
 * @tparam Values The types of the values to serialize. These do not need to match the Bundle.
 * @param output The output to append to at runtime.
 * @param values The values to serialize. This needs to match the number and order of @p Values.
 */
template <class Bundle, Output Out, typename... Values>
constexpr void serialize(Out &&output, Values... values) {
  return Bundle::serialize(output, values...);
}

template <class Bundle, Input In, typename... Values>
constexpr std::tuple<Values...> deserialize(In input) {
  return Bundle::template deserialize<In, Values...>(std::forward<In>(input));
}

/**
 * Shortcut to okser::serialize without the need to declare a bundle.
 *
 * ## Example
 * \code
 * okser::serialize<okser::sint<1>, okser::uint<2>>(output, 100, 50000);
 * \endcode
 *
 * @tparam Types The serializer types of the "bundle"
 * @todo See if this can be combined with the above function through deduction
 */
template <Serializer... Types, Output Out, typename... Values>
constexpr void serialize(Out &&output, Values... values) {
    return bundle<Types...>::serialize(output, values...);
}

template <Serializer... Types, Input In, typename... Values>
constexpr void deserialize(In input, Values... values) {
    return bundle<Types...>::deserialize(input, values...);
}

/**
 * Shortcut to okser::serialize that directly produces an std::string.
 *
 * ## Example
 * \code
 * std::string out = okser::simple_serialize<
 *     okser::sint<1>,
 *     okser::uint<2>
 * >(-5, 15);
 * \endcode
 */
template <Serializer... Types, typename... Values>
std::string simple_serialize(Values... values) {
    std::string output;
    bundle<Types...>::serialize(out::stdstring{output}, values...);
    return output;
}

} // namespace okser
