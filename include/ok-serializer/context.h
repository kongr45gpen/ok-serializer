#pragma once

#include "concepts.h"

namespace okser {

template<Input In>
struct input_context {
    In input;
    std::optional<okser::parse_error> error;
};

template<Output Out>
struct output_context {
    Out output;
};

}