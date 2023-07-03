#pragma once

#include "concepts.h"

namespace okser {

template<Input In>
struct input_context {
    In input;
    std::optional<okser::parse_error> error = std::nullopt;

    constexpr explicit input_context(In input) : input(input) {}
};

template<Output Out>
struct output_context {
    Out output;
};

}