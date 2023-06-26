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
template<class Pair, Output Out>
constexpr void serialize_one(Pair p, Out &&o) {
    Pair::SerializerType::serialize(p.value, o);
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

////todo: generalise to non-tuples
//template<typename F, typename ... T>
//constexpr auto apply(F f, std::tuple<T...> t) {
//    auto IndSeq = std::make_index_sequence<sizeof...(T)>{};
//
//    return std::tuple{f(std::get<IndSeq>(t))...};
//}

template<int Bytes>
using uint_bytes_to_type = std::conditional_t<Bytes <= 1, uint8_t,
        std::conditional_t<Bytes <= 2, uint16_t,
                std::conditional_t<Bytes <= 4, uint32_t,
                        std::conditional_t<Bytes <= 8, uint64_t,
                                void>>>>;

template<typename T, typename E, typename F>
constexpr auto transform(const std::expected<T, E> &e, F &&f) {
    using FunctionReturn = std::invoke_result_t<F, T>;
    using ExpectedReturn = std::expected<FunctionReturn, E>;

    if (e) {
        return ExpectedReturn(f(std::move(e.value())));
    } else {
        return ExpectedReturn(std::unexpected(std::move(e.error())));
    }
}

template<typename T>
concept HasConstIterator = requires()
{
    typename T::const_iterator;
};

template<typename T>
requires(HasConstIterator<T>)
typename T::const_iterator ConstIterator_s();

template<typename T>
requires(!HasConstIterator<T>)
typename T::iterator ConstIterator_s();

template<typename T>
using ConstIterator = decltype(ConstIterator_s<T>());



} // namespace internal

}