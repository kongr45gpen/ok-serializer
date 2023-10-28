#pragma once

#include <tuple>

#include "io.h"
#include "types_simple.h"
#include "concepts.h"
#include "utils_late.h"
#include "types_compound.h"

namespace okser {


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
template<class Bundle, Output Out, typename... Values>
[[nodiscard]] constexpr empty_result serialize(Out &&output, Values... values) {
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
//template<Serializer... Types, Output Out, typename... Values>
//constexpr void serialize(Out &&output, Values... values) {
//    return bundle<Types...>::serialize(output, values...);
//}

// Single argument deserialisation
template<Deserializer Type, typename Value = typename Type::DefaultType, class In>
constexpr result<Value> deserialize(In &&input) {
    auto contained_input = internal::convert_input_to_okser(std::forward<In>(input));
    input_context context(contained_input);

    return Type::template deserialize<Value>(context).first;
}

// Multiple argument deserialisation, converts many elements to bundles
template<Deserializer... Types, class... Values, std::derived_from<std::tuple<Values...>> Tuple = std::tuple<Values...>, class In>
requires (sizeof...(Values) == sizeof...(Types) && sizeof...(Types) > 1)
constexpr result<Tuple> deserialize(In &&input) {
    return deserialize<bundle<Tuple>, Values..., In>(std::forward<In>(input));
}

/**
 * Shortcut to okser::serialize that directly produces an std::string.
 *
 * ## Example
 * \code
 * std::string out = okser::serialize_to_string<
 *     okser::sint<1>,
 *     okser::uint<2>
 * >(-5, 15);
 * \endcode
 */
template<Serializer... Types, typename... Values, class Output = std::string>
Output serialize_to_string(Values... values) {
    Output output;
    [[maybe_unused]] auto status = bundle<Types...>::serialize(out::dynamic{output}, values...);
    return output;
}

} // namespace okser
