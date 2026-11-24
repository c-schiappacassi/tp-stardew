#pragma once

#include <chrono>
#include <thread>

class Foo {
public:
    template <typename Callback>
    static void constant_rate_loop(Callback&& func, std::chrono::milliseconds rate, bool& running);
};

#include "foo.tpp"