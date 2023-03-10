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
public:
  template <Output Out, typename... Values>
  constexpr static void serialize(Out &&output, Values... values) {
    std::tuple<serializable_value<Types, Values>...> typeValues{values...};

    std::apply(
        [&output](auto &&...v) { ((internal::serialize_one(v, output)), ...); },
        typeValues);
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
constexpr std::string simple_serialize(Values... values) {
    std::string output;
    bundle<Types...>::serialize(out::stdstring{output}, values...);
    return output;
}

} // namespace okser
