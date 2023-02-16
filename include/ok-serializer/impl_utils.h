#pragma once

namespace okser {
namespace internal {

/**
 * Append a single serializable_value to an output.
 * @tparam Pair The serializable_value type
 * @tparam Output The output type
 * @param p The serializable_value
 * @param o The output
 */
template<class Pair, class Output>
constexpr void serialize_one(Pair p, Output &&o) {
    Pair::SerializerType::serialize(p.value, o);
}

}
}