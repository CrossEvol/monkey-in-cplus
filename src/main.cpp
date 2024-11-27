#include <iostream>
#include <ostream>

#include "../cmake-build-debug-mingw/_deps/fmt-src/include/fmt/format.h"
#include "include/static/hello.h"

int main() {
    Hello hello;
    hello.print();
    std::cout << fmt::format("{:4d} {:8s} {:4d}",1,"hello",2) << std::endl;
    return 0;
}
