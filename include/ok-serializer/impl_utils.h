#pragma once

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


}
}