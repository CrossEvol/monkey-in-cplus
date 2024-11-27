#include "include/static/hello.h"
#include <iostream>

void Hello::print() {
    std::cout << "Hello, World" << std::endl;
}

std::string Hello::echo(std::string input) {
    return input;
}
