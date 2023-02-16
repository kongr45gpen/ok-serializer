#pragma once

#include <tuple>

#include "impl_outputs.h"
#include "impl_types.h"
#include "impl_utils.h"

namespace okser {

/**
 * A concept to check if a class can be used as an okser serializer and serialise values to binaries.
 *
 * By default, every child of the okser::internal::type class satisfies this concept. For any custom user-provided
 * serializable types, you can derive your type from okser::internal::type, or use a template specialization as follows:
 * \code
 * template<>
 * constexpr inline bool is_serializer<MySerializer> = true;
 * \endcode
 */
template<typename T>
concept Serializer = requires()
{
    requires internal::is_serializer<T>;
};


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
  template <class Output, typename... Values>
  constexpr static void serialize(Output &&output, Values... values) {
    std::tuple<serializable_value<Types, Values>...> typeValues{values...};

    std::apply(
        [&output](auto &&...v) { ((internal::serialize_one(v, output)), ...); },
        typeValues);
  }
};

/**
 * Take some @p values, serialize them using a @p bundle, and append them to an @p output
 * @tparam Bundle The bundle containing each element of the serialized structure.
 * @tparam Output The type of the output.
 * @tparam Values The types of the values to serialize. These do not need to match the Bundle.
 * @param output The output to append to at runtime.
 * @param values The values to serialize. This needs to match the number and order of @p Values.
 */
template <class Bundle, class Output, typename... Values>
constexpr void serialize(Output &&output, Values... values) {
  return Bundle::serialize(output, values...);
}

} // namespace okser
