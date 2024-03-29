#pragma once

#include <cstdint>
#include <array>
#include <string>
#include <cmath>
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

    template<std::integral V, OutputContext Context>
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

    /**
     * Unfortunately C++'s std::to_chars is not constexpr for floating point numbers, so we have to implement the
     * conversion ourselves.
     *
     * This is a barebones, wildly incorrect, double decimal digit approximation. There are many implementations online
     * to convert a floating point number to a string in a constexpr way.
     *
     * @todo Error handling
     */
    template<std::floating_point V, OutputContext Context>
    constexpr static empty_result serialize(Context &&output, V value) {
        constexpr int precision = 2;

        value *= 1.0000001f; // Fixes some rounding errors

        if (auto result = number::serialize(output, static_cast<int64_t>(value)); !result) {
            return result;
        }

        if (auto result = output->add('.'); !result) {
            return result;
        }

        for (int i = 0; i < precision; i++) {
            value -= static_cast<int64_t>(value);
            value *= 10;
            if (auto result = number::serialize(output, static_cast<int64_t>(value)); !result) {
                return result;
            }
        }

        return {};
    }
};

struct string : public okser::internal::type {
    using DefaultType = std::string;

    template<std::ranges::range S, OutputContext Context>
    constexpr static empty_result serialize(Context &&output, const S &string) {
        using namespace std::string_view_literals;

        if (auto result = output->add('"'); !result) {
            return result;
        }

        for (auto it: string) {
            empty_result result;
            if (it == '"') {
                result = output->add("\\\""sv);
            } else if (it == '\\') {
                result = output->add("\\\\"sv);
            } else if (it == '\t') {
                result = output->add("\\t"sv);
            } else if (it == '\r') {
                result = output->add("\\r"sv);
            } else if (it == '\n') {
                result = output->add("\\n"sv);
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
private:
    template<class Type, OutputContext Context>
    constexpr static empty_result serialize_element(Context &&output, const Type &value) {
        using Serializer = typename Configuration::template default_serializers<Type>::ser;
        return Serializer::template serialize(output, value);
    }

public:
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

            if (auto result = serialize_element<Type>(output, *it); !result) {
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

    template<class T, OutputContext Context>
    requires(!std::ranges::range<T>)
    constexpr static empty_result serialize(Context &&output, const T &tuple) {
        using namespace std::string_view_literals;

        if (auto result = output->add('['); !result) {
            return result;
        }

//        std::apply([&output](auto&&... args) {((serialize_element(output, args), output->add(", "sv)), ...);}, tuple);
        // TODO: Handle errors
        internal::apply([&output, &tuple](const auto i) -> empty_result {
            constexpr auto Index = decltype(i)::value;
            using Type = std::tuple_element_t<Index, T>;

            if (output.error) {
                return std::unexpected(*(output.error));
            }

            auto value = serialize_element<Type>(output, std::get<Index>(tuple));

            if (!value) {
                output.error = value.error();
                return value;
            }

            //TODO: replace with value.and_then
            if (i != std::tuple_size_v<T> - 1) {
                value = output->add(", "sv);
                if (!value) {
                    output.error = value.error();
                }
            }

            return value;
        }, std::make_index_sequence<std::tuple_size_v<T>>());

        return output->add(']');
    }
};

template<class Configuration = json_configuration>
struct object : public okser::internal::type {
    using DefaultType = struct {
    };

#if __cpp_reflection >= 201902L

    /**
     * @todo Error handling
     */
    template<class S, OutputContext Context>
    constexpr static empty_result serialize(Context &&output, const S &object) {
        using namespace std::string_view_literals;

        auto mirrored_struct = mirror(S);

        output->add('{');

        // TODO: This is an inexcusable hack to get the last element
        size_t last_id = 0;
        for_each(get_data_members(mirrored_struct), [&last_id](auto member) {
            last_id = get_id(member);
        });

        for_each(get_data_members(mirrored_struct), [&](auto member) {
            const auto &name = get_name(member);

            string::serialize(output, name);

            output->add(": "sv);

            const auto &value = get_value(member, object);
            using type = std::remove_cvref_t<decltype(value)>;

            using serializer = Configuration::template default_serializers<type>::ser;
            auto result = serializer::template serialize(output, value);

            if (get_id(member) != last_id) {
                output->add(", "sv);
            }
        });

        return output->add('}');
    }

#else
    template<class S, OutputContext Context>
    constexpr static empty_result serialize(Context &&output, const S &object) {
        // not implemented without reflection
        return {};
    }
#endif
};

/**
 * @todo Add tuples and pairs
 */
struct json_configuration : okser::configuration<> {
    template<class T>
    struct default_serializers {
        using ser = object<>;
    };

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

    template<std::ranges::range T> requires (!std::derived_from<std::string, T>)
    struct default_serializers<T> {
        using ser = array<>;
    };
};

} // namespace json
} // namespace okser