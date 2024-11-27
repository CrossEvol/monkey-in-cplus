#include <catch2/catch_test_macros.hpp>
#include  "../src/include/static/hello.h"
#define CATCH_CONFIG_MAIN


unsigned int Factorial(const unsigned int number) {
    return number <= 1 ? number : Factorial(number - 1) * number;
}

TEST_CASE("Factorials are computed", "[factorial]") {
    REQUIRE(Factorial(1) == 1);
    REQUIRE(Factorial(2) == 2);
    REQUIRE(Factorial(3) == 6);
    REQUIRE(Factorial(10) == 3628800);
}

TEST_CASE("Echo returns input string", "[echo]") {
    Hello hello;
    REQUIRE(hello.echo("test") == "test");
    REQUIRE(hello.echo("hello world") == "hello world");
    REQUIRE(hello.echo("").empty());
}