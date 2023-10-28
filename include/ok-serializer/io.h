#pragma once

#include <functional>
#include <string>
#include <ranges>
#include <iterator>
#include <expected>

#include "errors.h"
#include "utils_early.h"

namespace okser {
/**
 * Useful output classes for serialized results
 */
namespace in {

/**
 * Input for data to be deserialized from any std-compatible range.
 *
 * ## Example
 * \code
 * std::string input = "Hello world";
 * auto range = okser::in::range{input};
 *
 * // If you have a C-style buffer and want to read some bytes off it
 * char buffer[1000] = ...
 * auto range = okser::in::range(std::span(buffer + 400, buffer + 500)); // or
 * auto range = okser::in::range(std::span(buffer + 400, 100));
 * \endcode
 *
 * @tparam R The input range type. It can be a string, an STL container, an std::span, or any other range.
 */
template<std::ranges::input_range R = std::string>
class range {
private:
    using Const_Iterator = decltype(std::ranges::cbegin(std::declval<R&>()));

    Const_Iterator begin;
    Const_Iterator end;

    constexpr range(Const_Iterator begin, Const_Iterator end) : begin(begin), end(end) {}

public:
    using ContainedType = R;

    /**
     * Construct a range input from a container.
     *
     * You can use std::span to control the range of the container to be used more accurately.
     */
    constexpr range(const R &_range) : begin(_range.begin()), end(_range.end()) {}

    /**
     * Get a single byte from the input.
     *
     * Returns okser::error_type::not_enough_input_bytes if the input range runs out.
     *
     * @return A pair with the byte and a new range with this byte taken away.
     */
    constexpr std::pair<okser::result<uint8_t>, range<R>> get() const {
        okser::result<uint8_t> result = std::unexpected(okser::error_type::not_enough_input_bytes);

        if (begin != end) {
            result = *begin;
        }

        return {result, range<R>{begin + 1, end}};
    }

    /**
     * Get a number of bytes (specified at compile-time) from the input.
     *
     * Returns okser::error_type::not_enough_input_bytes if the input range runs out.
     *
     * @tparam N The number of bytes to get
     * @tparam Array The container to store the output in
     * @return A pair with the Array and a new range with the bytes taken away.
     */
    template<size_t N, std::ranges::input_range Array = std::array<uint8_t, N>>
    requires (N > 0, std::tuple_size_v<Array> >= N)
    constexpr std::pair<okser::result<Array>, range<R>> get() const {
        okser::result<Array> result = std::unexpected(okser::error_type::not_enough_input_bytes);
        if (std::ranges::distance(begin, end) >= N) {
            result.emplace();
            std::copy(begin, begin + N, result->begin());
        }
        return {result, range<R>{begin + N, end}};
    }

    /**
     * Get a number of bytes (specified at runtime) from the input.
     *
     * Returns okser::error_type::not_enough_input_bytes if the input range runs out.
     *
     * @param N The number of bytes to get
     * @tparam Vector The container to store the output in
     * @return A pair with the Array and a new range with the bytes taken away.
     */
    template<std::ranges::input_range Vector = std::string>
    constexpr std::pair<okser::result<Vector>, range<R>> get(size_t N) const {
        okser::result<Vector> result = std::unexpected(okser::error_type::not_enough_input_bytes);
        auto current = begin;
        if (std::ranges::distance(begin, end) >= N) {
            result.emplace();
            for (int i = 0; i < N; i++) {
                result->push_back(*current);
                current++;
            }
        }
        return {result, range<R>{current, end}};
    }
};
}

/**
 * Useful output classes for serialized results
 */
namespace out {
/**
 * An output to a dynamic C++ container, such as an std::vector or an std::string
 *
 * ## Example
 * \code
 * std::string result;
 * auto output = okser::out::dynamic{result};
 *
 * // ...
 *
 * okser::serialize<bundle>(output, 100, 200, 300);
 * \endcode
 */
template<class String = std::string>
class dynamic {
public:
    std::reference_wrapper<String> str;

    explicit dynamic(String &str) : str(str) {}

    /**
     * Add one or more bytes to the output
     * @tparam T The type of value to add.
     * @param value The value to append to the output
     */
    template<typename T>
    empty_result add(const T &value) {
        str.get().append(value);

        return {};
    }

    /**
     * Template specialisation to a single character to the output
     */
    empty_result add(const unsigned char &value) {
        str.get().push_back(value);

        return {};
    }

    /**
     * Template specialisation to a single character to the output
     */
    empty_result add(const char &value) {
        str.get().push_back(value);

        return {};
    }
};

/**
 * Alias for okser::out::dynamic<std::string>
 */
using stdstring = dynamic<std::string>;

/**
 * An output to a pre-allocated C++ range, such as an std::array
 *
 * ## Example
 * \code
 * std::array<uint8_t, 3> result;
 * auto output = okser::out::fixed_container{result};
 *
 * // ...
 *
 * okser::serialize<bundle>(output, 100, 200, 300);
 * \endcode
 *
 * @note This only works with containers that have a certain number of elements preallocated. For example,
 * an empty vector with 0 elements will thrown an error, while a vector with N elements will have those elements
 * replaced.
 */
template<std::ranges::output_range<uint8_t> C> requires (std::ranges::forward_range<C>)
class fixed_container {
private:
    typename C::iterator current;
    typename C::const_iterator last;
public:
    explicit fixed_container(C &container) : current(container.begin()), last(container.end()) {}

    /**
     * Add a single byte to the output
     * @tparam T
     * @param value
     */
    template<typename T>
    empty_result add(const T &value) {
        if (current == last) {
            return std::unexpected(error_type::not_enough_output_bytes);
        }
        *current = value;
        current++;

        return {};
    }
};

/**
 * A simple output to a C-style char buffer
 *
 * @warning This class does not check if the size of the buffer is enough to hold the serialized data.
 * The responsibility falls on the user to ensure that the serialized data cannot theoretically overflow.
 *
 * ## Example
 * \code
 * char buffer[100];
 * auto output = okser::out::cbuf{buffer};
 *
 * // ...
 *
 * okser::serialize<bundle>(output, 100, 200, 300);
 * size_t length = output.size;
 * \endcode
 */
class cbuf {
public:
    char *buf;

    /**
     * The number of bytes appended to the buffer so far
     */
    size_t size = 0;

    /**
     * Add a single character to the buffer
     */
    empty_result add(char value) {
        buf[size] = value;
        size++;

        return {};
    }

    /**
     * Add a single character to the buffer
     */
    empty_result add(uint8_t value) {
        buf[size] = value;
        size++;

        return {};
    }
};
}
}
