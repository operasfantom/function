#include "function.hpp"

#include <iostream>

void print(std::ostream &os, int x) {
    os << x;
}

auto print_ptr = &print;

int main() {
//    function<void(int x)> f;
    int y = 2;
    function<void(std::ostream &os, int x)> f;
    auto lambda = [y](std::ostream &os, int x) mutable {
        os << x << ' ' << y << std::endl;
    };
    f = lambda;
    f(std::cout, 1);

    f = print_ptr;
    f(std::cout, 2);

    std::vector<int> large_data(1000, -1);
    function<void(std::ostream &)> g = [large_data](std::ostream &os) {
        os << "LARGE:" << large_data[0] << std::endl;
    };
    g(std::cerr);
}