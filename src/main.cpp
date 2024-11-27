#include <iostream>
#include <ostream>
#include <vector>

#include "../cmake-build-debug-mingw/_deps/fmt-src/include/fmt/format.h"
#include "include/static/hello.h"

class A {

};

class B :public  A {};

class C :public  A {};

class D :public  A {};

int main() {
    Hello hello;
    hello.print();
    std::vector<std::shared_ptr<A>> list {};
    list.push_back(std::make_shared<B>());
    list.push_back(std::make_shared<C>());
    D d{};
    auto move = std::move(d);
    list.emplace_back(std::make_shared<A>(std::move(d)));
    if(typeid(d) == typeid(D)) {
        std::cout << typeid(d).name() << std::endl;
    }
    return 0;
}
