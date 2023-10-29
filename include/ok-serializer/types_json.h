#pragma once

#include <cstdint>
#include <array>
#include <string>
#include "types_simple.h"
#include "reflection.h"

/**
 * @defgroup simple_types Simple Types
 */
namespace okser {
namespace json {

struct json_configuration;

struct boolean : public okser::internal::type {
    using DefaultType = boolean;

    template<typename V, OutputContext Context>
    constexpr static empty_result serialize(Context &&output, const V &value) {
        using namespace std::string_view_literals;
        if (value) {
            return output->add("true"sv);
        }
        return output->add("false"sv);
    }
};

struct null : public okser::internal::type {
    using DefaultType = nullptr_t;

    template<typename V, OutputContext Context>
    constexpr static empty_result serialize(Context &&output, const V &value) {
        using namespace std::string_view_literals;
        return output->add("null"sv);
    }
};

struct number : public okser::internal::type {
    using DefaultType = int64_t;

    template<typename V, OutputContext Context>
    constexpr static empty_result serialize(Context &&output, V value) {
        // A 64-bit integer can be represented in 20 characters max (hopefully)
        std::array<char, 20> buffer{'\0'};

        // std::to_chars is the only constexpr way to convert an integer to string in STL
        auto result = std::to_chars(buffer.begin(), buffer.end(), value);
        if (result.ec != std::errc()) {
            return std::unexpected(error_type::io_error);
        }

        return output->add(std::string_view(buffer.data(), result.ptr));
    }
};

struct string : public okser::internal::type {
    using DefaultType = std::string;

    template<std::ranges::range S, OutputContext Context>
    constexpr static empty_result serialize(Context &&output, const S &string) {
        using namespace std::string_literals;

        if (auto result = output->add('"'); !result) {
            return result;
        }

        for (auto it: string) {
            empty_result result;
            if (it == '"') {
                result = output->add("\\\"");
            } else if (it == '\\') {
                result = output->add("\\\\");
            } else if (it == '\t') {
                result = output->add("\\t");
            } else if (it == '\r') {
                result = output->add("\\r");
            } else if (it == '\n') {
                result = output->add("\\n");
            } else if (it < ' ' || it > '~') {
                constexpr char hex[] = "0123456789ABCDEF";
                std::array<char, 4> buffer{
                        '\\', 'x', hex[(it >> 4) & 0xF], hex[it & 0xF]
                };
                result = output->add(std::string_view(buffer.begin(), buffer.end()));
            } else {
                result = output->add(it);
            }

            if (!result) {
                return result;
            }
        }

        return output->add('"');
    }
};

template<class Configuration = json_configuration>
struct array : public okser::internal::type {
    using DefaultType = std::string;

    /**
     * @todo Check implementation for different containers
     * @todo Check implementation for maps
     */
    template<std::ranges::range R, OutputContext Context>
    constexpr static empty_result serialize(Context &&output, const R &range) {
        using namespace std::string_view_literals;

        if (auto result = output->add('['); !result) {
            return result;
        }

        for (auto it = range.cbegin(); it != range.cend(); it++) {
            using Type = std::remove_cvref_t<std::ranges::range_value_t<R>>;
            using Serializer = typename Configuration::template default_serializers<Type>::ser;

            if (auto result = Serializer::template serialize(output, *it); !result) {
                return result;
            }

            if (it != range.cend() - 1) {
                if (auto result = output->add(", "sv); !result) {
                    return result;
                }
            }
        }

        return output->add(']');
    }
};

struct json_configuration : okser::configuration<> {
    template<class T>
    struct default_serializers;

    template<>
    struct default_serializers<bool> {
        using ser = boolean;
    };

    template<>
    struct default_serializers<nullptr_t> {
        using ser = null;
    };

    template<std::integral T>
    struct default_serializers<T> {
        using ser = number;
    };

    template<std::floating_point T>
    struct default_serializers<T> {
        using ser = number;
    };

    template<std::derived_from<std::string> T>
    struct default_serializers<T> {
        using ser = string;
    };
};

} // namespace json
} // namespace okser