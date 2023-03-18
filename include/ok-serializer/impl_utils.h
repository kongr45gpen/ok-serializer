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

template <typename F, std::size_t ... Is>
constexpr auto apply(F f, std::index_sequence<Is...>)
    -> std::tuple<decltype(f(std::integral_constant<std::size_t, Is>{}...))>
{
    return std::make_tuple(f(std::integral_constant<std::size_t, Is...>{}));
}

template<int Bytes>
using uint_bytes_to_type = std::conditional_t<Bytes <= 1, uint8_t,
        std::conditional_t<Bytes <= 2, uint16_t,
                std::conditional_t<Bytes <= 4, uint32_t,
                        std::conditional_t<Bytes <= 8, uint64_t,
                                void>>>>;

} // namespace internal

}