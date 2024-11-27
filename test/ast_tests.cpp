#include <catch2/catch_test_macros.hpp>
#include  "../src/ast/ast.h"
#include  "../src/token/token.h"
#define CATCH_CONFIG_MAIN
#include <iostream>


TEST_CASE("test ast string()", "[ast]") {
    const auto letStatement = new LetStatement(Token{LET, "let"},
                                               new Identifier({Token{IDENT, "myVar"}, "myVar"}),
                                               new Identifier({Token{IDENT, "anotherVar"}, "anotherVar"}));
    const std::string expected = "let myVar = anotherVar;";
    auto actual = letStatement->string();
    INFO("Expected: '" << expected << "'");
    INFO("Actual: '" << actual << "'");
    REQUIRE(actual == expected);
}
