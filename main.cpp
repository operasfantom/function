#include "function.hpp"

#include <iostream>

void print(std::ostream &os, int x) {
    os << x;
}

auto print_ptr = &print;

int main() {
//    function<void(int x)> f;
    function<void(std::ostream &os, int x)> f;
    auto lambda = [](std::ostream &os, int x) mutable {
        os << x << std::endl;
    };
    f = lambda;
    f(std::cout, 1);

    f = print_ptr;
    f(std::cout, 2);
}