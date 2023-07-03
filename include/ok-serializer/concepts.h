#pragma once

#include <optional>
#include "errors.h"

/**
 * @file
 * @brief A file containing various concepts used by ok-serializer, to specify restrictions on template parameters.
 */

namespace okser {

namespace internal {

/**
 * An representative input/output that inputs nowhere and outputs nowhere.
 * To be used as a concept argument.
 */
struct DummyInOut {
    constexpr void add(uint8_t) const {}

    constexpr std::pair<std::optional<uint8_t>, DummyInOut> get() const {
        return {std::nullopt, DummyInOut{}};
    }
};

/**
 * A representative context that includes the dummy input and output.
 * To be used as a concept argument.
 */
struct DummyContext {
    DummyInOut input;
    DummyInOut output;
    std::optional<okser::parse_error> error;
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

/**
 * A concept to check if a class can be used as an okser deserializer, and deserialise values from binaries.
 * @see Serializer
 */
template<typename T>
concept Deserializer = requires(typename T::DefaultType v, internal::DummyContext c)
{
//    T::template deserialize<typename T::DefaultType>(c);
    1; //TODO
};

/**
 * A concept to check if a class can be used as an okser input context.
 *
 * An input context needs to contain at least an okser::Input. Useful for de-serializing.
 */
template<typename T>
concept InputContext = requires(T t)
{
    t.input;
};

/**
 * A concept to check if a class can be used as an okser output context.
 *
 * An output context needs to contain at least an okser::Output. Useful for serializing.
 */
template<typename T>
concept OutputContext = requires(T t)
{
    t.output;
};

}