#pragma once

#include <utility>
#include "concepts.h"

namespace okser {
namespace internal {
/**
 * Append a single serializable_value to an output.
 * @tparam Pair The serializable_value type
 * @tparam Out The output type
 * @param p The serializable_value
 * @param o The output
 */
template<class Pair, OutputContext Context>
constexpr void serialize_one(Context &&out, Pair p) {
    Pair::SerializerType::serialize(out, p.value);
}

/**
 * Apply a function to all elements of an index sequence.
 * This function is useful for looping over tuples at compile time with an index. The applications would be looping
 * over two tuples at the same time.
 * @tparam F The function to apply. The return value of this function will be returned as a tuple. The only argument
 *           passed to this function is an std::integral_constant with the index.
 * @tparam Is An index sequence.
 * @param f The function to apply.
 * @return An std::tuple with the elements of each application of f.
 */
template<typename F, std::size_t ... Is>
constexpr auto apply(F f, std::index_sequence<Is...>) {
    // We cannot use packed parameter expansion, as the evaluation order might be random.
    // So we use braced initialiser lists.
    return std::tuple{f(std::integral_constant<std::size_t, Is>{})...};
}

template<class R>
concept dynamic_range = requires(R r)
{
    std::ranges::range<R>;
    { r.push_back(*std::ranges::begin(r)) };
};

/**
 * Get an iterator for a range. If the range is sized, returns a normal iterator to the beginning of the range.
 * Otherwise, returns an [std::back_insert_iterator](https://en.cppreference.com/w/cpp/iterator/back_insert_iterator)
 * that will append new elements to the range when assigned to.
 * @tparam R The range type
 * @param input The range to get the iterators from
 * @return An std::iterator
 */
template<std::ranges::range R>
constexpr auto get_fixed_or_dynamic_iterator(auto &input) {
    if constexpr (dynamic_range<R>) {
        return std::back_inserter(input);
    } else {
        return std::ranges::begin(input);
    }
}

template<std::ranges::range R, class I>
requires (std::incrementable<I>)
constexpr bool is_range_end(const R &range, const I &it) {
    return it == std::ranges::end(range);
}

template<std::ranges::range R, class I>
requires (!std::incrementable<I>)
constexpr bool is_range_end(const R &, const I &) {
    return false;
}

/**
 * Convert from a number of bytes to the smallest unsigned integer type that can hold that many bytes.
 */
template<int Bytes>
using uint_bytes_to_type = std::conditional_t<Bytes <= 1, uint8_t,
        std::conditional_t<Bytes <= 2, uint16_t,
                std::conditional_t<Bytes <= 4, uint32_t,
                        std::conditional_t<Bytes <= 8, uint64_t,
                                void>>>>;

} // namespace internal

}