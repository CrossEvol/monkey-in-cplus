#include <iostream>
#include <ostream>
#include <vector>

#include "../cmake-build-debug-mingw/_deps/fmt-src/include/fmt/format.h"
#include "include/static/hello.h"

int main() {
    Hello hello;
    hello.print();
    return 0;
}
