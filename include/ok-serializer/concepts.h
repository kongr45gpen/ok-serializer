#pragma once

namespace okser {

namespace internal {

struct DummyInOut {
    constexpr void add(uint8_t) const {}

    constexpr std::pair<std::optional<uint8_t>, DummyInOut> get() const {
        return {std::nullopt, DummyInOut{}};
    }
};

}

/**
 * A concept to check if a class can be used as an okser output.
 */
template<typename T>
concept Output = requires(T t, uint8_t byte)
{
    { t.add(byte) };
};

/**
 * A concept to check if a class can be used as an okser input.
 */
template<typename T>
concept Input = requires(T t)
{
    { t.get() };
};

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
concept Serializer = requires(typename T::DefaultType v, internal::DummyInOut o)
{
    T::serialize(v, o);
};

template<typename T>
concept Deserializer = requires(typename T::DefaultType v, internal::DummyInOut i)
{
    T::template deserialize<typename T::DefaultType>(i);
};

}