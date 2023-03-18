#pragma once

namespace okser {

template<typename B>
concept IsBundle = requires(B b)
{
    B::i_am_a_bundle == true;
};

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
template <typename F, std::size_t ... Is>
constexpr auto apply(F f, std::index_sequence<Is...>) {
    // We cannot use packed parameter expansion, as the evaluation order might be random.
    // So we use braced initialiser lists.
    return std::tuple{f(std::integral_constant<std::size_t, Is>{})...};
}

template<int Bytes>
using uint_bytes_to_type = std::conditional_t<Bytes <= 1, uint8_t,
        std::conditional_t<Bytes <= 2, uint16_t,
                std::conditional_t<Bytes <= 4, uint32_t,
                        std::conditional_t<Bytes <= 8, uint64_t,
                                void>>>>;

} // namespace internal

}