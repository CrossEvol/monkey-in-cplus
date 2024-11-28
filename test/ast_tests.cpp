#include <catch2/catch_test_macros.hpp>
#include  "../src/ast/ast.h"
#include  "../src/token/token.h"
#define CATCH_CONFIG_MAIN
#include <iostream>

TEST_CASE("test ast string()", "[ast]") {
    using namespace Ast;

    auto letStatement = std::make_unique<LetStatement>(
        Token{LET, "let"},
        std::make_unique<Identifier>(
            Token{IDENT, "myVar"},
            "myVar"
        ),
        std::make_unique<Identifier>(
        Token{IDENT, "anotherVar"},
            "anotherVar"
        )
    );

    const std::string expected = "let myVar = anotherVar;";
    auto actual = letStatement->string();
    INFO("Expected: '" << expected << "'");
    INFO("Actual: '" << actual << "'");
    REQUIRE(actual == expected);
}
