#pragma once

#include <any>
#include <expected>
#include <memory>

namespace okser {

enum class error_type : uint8_t {
    not_enough_bytes,
    io_error
};

struct parse_error {
    error_type type;
//    std::optional<std::reference_wrapper<std::any>> error;

    explicit(false) constexpr parse_error(error_type type) noexcept: type(type) {}

    error_type operator()() const noexcept {
        return type;
    }

    error_type operator*() const noexcept {
        return type;
    }
};

template<class T>
using result = std::expected<T, parse_error>;

}