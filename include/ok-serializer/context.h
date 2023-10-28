#pragma once

#include "concepts.h"

namespace okser {

/**
 * The input context contains gritty details about the input, errors, and other parsing information about what is
 * happening at the input.
 *
 * It is also used internally to allow looping at compile-time, so that one step in std::apply knows if the previous
 * step was executed correctly, or if it should stop processing the input.
 */
template<Input In>
struct input_context {
    In input;
    std::optional<okser::parse_error> error = std::nullopt;

    constexpr explicit input_context(In input) : input(input) {}

    constexpr In *operator->() {
        return &input;
    }
};

/**
 * The output context contains gritty details about the input, errors, and other parsing information about what is
 * happening at the input.
 *
 * It is also used internally to allow looping at compile-time, so that one step in std::apply knows if the previous
 * step was executed correctly, or if it should stop processing the output.
 */
template<Output Out>
struct output_context {
    Out output;
    std::optional<okser::parse_error> error = std::nullopt;

    constexpr explicit output_context(Out output) : output(output) {}

    constexpr Out *operator->() {
        return &output;
    }
};

}