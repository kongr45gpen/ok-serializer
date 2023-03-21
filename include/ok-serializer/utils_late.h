#pragma once

#include <type_traits>
#include "io.h"

namespace okser {
namespace internal {

template<class In>
requires(std::derived_from<In, okser::in::range<typename In::ContainedType>>)
auto constexpr convert_input_to_okser(const In& unknown_input) {
    return unknown_input;
}

template<class In>
requires(std::constructible_from<okser::in::range<In>, In>)
auto constexpr convert_input_to_okser(const In& unknown_input) {
    return okser::in::range(unknown_input);
}

} // namespace internal
}