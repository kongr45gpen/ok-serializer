#pragma once

namespace okser {

template<typename T>
concept InputContext = requires(T t)
{
    t.input;
};

template<typename T>
concept OutputContext = requires(T t)
{
    t.output;
};


}