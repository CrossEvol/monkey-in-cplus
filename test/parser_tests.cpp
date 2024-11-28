#include <variant>
#include <catch2/catch_test_macros.hpp>
#include "../src/parser/parser.h"
#include "../src/lexer/lexer.h"
#include "../src/ast/ast.h"

// Helper functions for testing
void checkParserErrors(Parser &p) {
    auto errors = p.errors();
    if (errors.empty()) {
        return;
    }

    FAIL("parser has " + std::to_string(errors.size()) + " errors:\n" +
        [&errors]() {
        std::string result;
        for (const auto& err : errors) {
        result += "\t" + err + "\n";
        }
        return result;
        }());
}

bool testLetStatement(Ast::Statement *stmt, const std::string &name) {
    REQUIRE(stmt->tokenLiteral() == "let");

    auto letStmt = dynamic_cast<Ast::LetStatement *>(stmt);
    REQUIRE(letStmt != nullptr);
    REQUIRE(letStmt->name->value == name);
    REQUIRE(letStmt->name->tokenLiteral() == name);

    return true;
}

bool testIntegerLiteral(Ast::Expression *exp, int64_t value) {
    auto integ = dynamic_cast<Ast::IntegerLiteral *>(exp);
    REQUIRE(integ != nullptr);
    REQUIRE(integ->value == value);
    REQUIRE(integ->tokenLiteral() == std::to_string(value));
    return true;
}

bool testIdentifier(Ast::Expression *exp, const std::string &value) {
    auto ident = dynamic_cast<Ast::Identifier *>(exp);
    REQUIRE(ident != nullptr);
    REQUIRE(ident->value == value);
    REQUIRE(ident->tokenLiteral() == value);
    return true;
}

bool testBooleanLiteral(Ast::Expression *exp, bool value) {
    auto boolean = dynamic_cast<Ast::Boolean *>(exp);
    REQUIRE(boolean != nullptr);
    REQUIRE(boolean->value == value);
    REQUIRE(boolean->tokenLiteral() == (value ? "true" : "false"));
    return true;
}

TEST_CASE("Test let statements", "[parser]") {
    struct Test {
        std::string input;
        std::string expectedIdentifier;
        std::variant<int64_t, bool, std::string> expectedValue;
    };

    std::vector<Test> tests = {
        {"let x = 5;", "x", int64_t(5)},
        {"let y = true;", "y", true},
        {"let foobar = y;", "foobar", std::string("y")}
    };

    for (const auto &tt: tests) {
        Lexer l(tt.input);
        Parser p(std::move(l));
        auto program = p.parseProgram();
        checkParserErrors(p);

        REQUIRE(program->statements.size() == 1);
        auto stmt = program->statements[0].get();
        REQUIRE(testLetStatement(stmt, tt.expectedIdentifier));
    }
}

TEST_CASE("Test return statements", "[parser]") {
    struct Test {
        std::string input;
        std::variant<int64_t, bool, std::string> expectedValue;
    };

    std::vector<Test> tests = {
        {"return 5;", int64_t(5)},
        {"return true;", true},
        {"return foobar;", std::string("foobar")}
    };

    for (const auto &tt: tests) {
        Lexer l(tt.input);
        Parser p(std::move(l));
        auto program = p.parseProgram();
        checkParserErrors(p);

        REQUIRE(program->statements.size() == 1);

        auto returnStmt = dynamic_cast<Ast::ReturnStatement *>(program->statements[0].get());
        REQUIRE(returnStmt != nullptr);
        REQUIRE(returnStmt->tokenLiteral() == "return");
    }
}

TEST_CASE("Test identifier expression", "[parser]") {
    const std::string input = "foobar;";

    Lexer l(input);
    Parser p(std::move(l));
    auto program = p.parseProgram();
    checkParserErrors(p);

    REQUIRE(program->statements.size() == 1);

    auto stmt = dynamic_cast<Ast::ExpressionStatement *>(program->statements[0].get());
    REQUIRE(stmt != nullptr);

    auto ident = dynamic_cast<Ast::Identifier *>(stmt->expression.get());
    REQUIRE(ident != nullptr);
    REQUIRE(ident->value == "foobar");
    REQUIRE(ident->tokenLiteral() == "foobar");
}

TEST_CASE("Test integer literal expression", "[parser]") {
    const std::string input = "5;";

    Lexer l(input);
    Parser p(std::move(l));
    auto program = p.parseProgram();
    checkParserErrors(p);

    REQUIRE(program->statements.size() == 1);

    auto stmt = dynamic_cast<Ast::ExpressionStatement *>(program->statements[0].get());
    REQUIRE(stmt != nullptr);

    testIntegerLiteral(stmt->expression.get(), 5);
}

TEST_CASE("Test parsing prefix expressions", "[parser]") {
    struct Test {
        std::string input;
        std::string operator_;
        std::variant<int64_t, bool, std::string> value;
    };

    std::vector<Test> tests = {
        {"!5;", "!", int64_t(5)},
        {"-15;", "-", int64_t(15)},
        {"!foobar;", "!", std::string("foobar")},
        {"-foobar;", "-", std::string("foobar")},
        {"!true;", "!", true},
        {"!false;", "!", false}
    };

    for (const auto &tt: tests) {
        Lexer l(tt.input);
        Parser p(std::move(l));
        auto program = p.parseProgram();
        checkParserErrors(p);

        REQUIRE(program->statements.size() == 1);

        auto stmt = dynamic_cast<Ast::ExpressionStatement *>(program->statements[0].get());
        REQUIRE(stmt != nullptr);

        auto exp = dynamic_cast<Ast::PrefixExpression *>(stmt->expression.get());
        REQUIRE(exp != nullptr);
        REQUIRE(exp->operator_ == tt.operator_);

        // Test the right expression based on the expected value type
        std::visit([&](auto &&arg) {
            using T = std::decay_t<decltype(arg)>;
            if constexpr (std::is_same_v<T, int64_t>) {
                testIntegerLiteral(exp->right.get(), arg);
            } else if constexpr (std::is_same_v<T, bool>) {
                testBooleanLiteral(exp->right.get(), arg);
            } else if constexpr (std::is_same_v<T, std::string>) {
                testIdentifier(exp->right.get(), arg);
            }
        }, tt.value);
    }
}

TEST_CASE("Test parsing infix expressions", "[parser]") {
    struct Test {
        std::string input;
        std::variant<int64_t, bool, std::string> leftValue;
        std::string operator_;
        std::variant<int64_t, bool, std::string> rightValue;
    };

    std::vector<Test> tests = {
        {"5 + 5;", int64_t(5), "+", int64_t(5)},
        {"5 - 5;", int64_t(5), "-", int64_t(5)},
        {"5 * 5;", int64_t(5), "*", int64_t(5)},
        {"5 / 5;", int64_t(5), "/", int64_t(5)},
        {"5 > 5;", int64_t(5), ">", int64_t(5)},
        {"5 < 5;", int64_t(5), "<", int64_t(5)},
        {"5 == 5;", int64_t(5), "==", int64_t(5)},
        {"5 != 5;", int64_t(5), "!=", int64_t(5)},
        {"foobar + barfoo;", std::string("foobar"), "+", std::string("barfoo")},
        {"foobar - barfoo;", std::string("foobar"), "-", std::string("barfoo")},
        {"foobar * barfoo;", std::string("foobar"), "*", std::string("barfoo")},
        {"foobar / barfoo;", std::string("foobar"), "/", std::string("barfoo")},
        {"foobar > barfoo;", std::string("foobar"), ">", std::string("barfoo")},
        {"foobar < barfoo;", std::string("foobar"), "<", std::string("barfoo")},
        {"foobar == barfoo;", std::string("foobar"), "==", std::string("barfoo")},
        {"foobar != barfoo;", std::string("foobar"), "!=", std::string("barfoo")},
        {"true == true", true, "==", true},
        {"true != false", true, "!=", false},
        {"false == false", false, "==", false}
    };

    for (const auto &tt: tests) {
        Lexer l(tt.input);
        Parser p(std::move(l));
        auto program = p.parseProgram();
        checkParserErrors(p);

        REQUIRE(program->statements.size() == 1);

        auto stmt = dynamic_cast<Ast::ExpressionStatement *>(program->statements[0].get());
        REQUIRE(stmt != nullptr);

        auto exp = dynamic_cast<Ast::InfixExpression *>(stmt->expression.get());
        REQUIRE(exp != nullptr);
        REQUIRE(exp->operator_ == tt.operator_);

        // Test left expression
        std::visit([&](auto &&arg) {
            using T = std::decay_t<decltype(arg)>;
            if constexpr (std::is_same_v<T, int64_t>) {
                testIntegerLiteral(exp->left.get(), arg);
            } else if constexpr (std::is_same_v<T, bool>) {
                testBooleanLiteral(exp->left.get(), arg);
            } else if constexpr (std::is_same_v<T, std::string>) {
                testIdentifier(exp->left.get(), arg);
            }
        }, tt.leftValue);

        // Test right expression
        std::visit([&](auto &&arg) {
            using T = std::decay_t<decltype(arg)>;
            if constexpr (std::is_same_v<T, int64_t>) {
                testIntegerLiteral(exp->right.get(), arg);
            } else if constexpr (std::is_same_v<T, bool>) {
                testBooleanLiteral(exp->right.get(), arg);
            } else if constexpr (std::is_same_v<T, std::string>) {
                testIdentifier(exp->right.get(), arg);
            }
        }, tt.rightValue);
    }
}

TEST_CASE("Test operator precedence parsing", "[parser]") {
    struct Test {
        std::string input;
        std::string expected;
    };

    std::vector<Test> tests = {
        {"-a * b", "((-a) * b)"},
        {"!-a", "(!(-a))"},
        {"a + b + c", "((a + b) + c)"},
        {"a + b - c", "((a + b) - c)"},
        {"a * b * c", "((a * b) * c)"},
        {"a * b / c", "((a * b) / c)"},
        {"a + b / c", "(a + (b / c))"},
        {"a + b * c + d / e - f", "(((a + (b * c)) + (d / e)) - f)"},
        {"3 + 4; -5 * 5", "(3 + 4)((-5) * 5)"},
        {"5 > 4 == 3 < 4", "((5 > 4) == (3 < 4))"},
        {"5 < 4 != 3 > 4", "((5 < 4) != (3 > 4))"},
        {"3 + 4 * 5 == 3 * 1 + 4 * 5", "((3 + (4 * 5)) == ((3 * 1) + (4 * 5)))"},
        {"true", "true"},
        {"false", "false"},
        {"3 > 5 == false", "((3 > 5) == false)"},
        {"3 < 5 == true", "((3 < 5) == true)"},
        {"1 + (2 + 3) + 4", "((1 + (2 + 3)) + 4)"},
        {"(5 + 5) * 2", "((5 + 5) * 2)"},
        {"2 / (5 + 5)", "(2 / (5 + 5))"},
        {"(5 + 5) * 2 * (5 + 5)", "(((5 + 5) * 2) * (5 + 5))"},
        {"-(5 + 5)", "(-(5 + 5))"},
        {"!(true == true)", "(!(true == true))"},
        {"a + add(b * c) + d", "((a + add((b * c))) + d)"},
        {
            "add(a, b, 1, 2 * 3, 4 + 5, add(6, 7 * 8))",
            "add(a, b, 1, (2 * 3), (4 + 5), add(6, (7 * 8)))"
        },
        {
            "add(a + b + c * d / f + g)",
            "add((((a + b) + ((c * d) / f)) + g))"
        },
        {
            "a * [1, 2, 3, 4][b * c] * d",
            "((a * ([1, 2, 3, 4][(b * c)])) * d)"
        },
        {
            "add(a * b[2], b[1], 2 * [1, 2][1])",
            "add((a * (b[2])), (b[1]), (2 * ([1, 2][1])))"
        }
    };

    for (const auto &tt: tests) {
        SECTION(tt.input) {
            Lexer l(tt.input);
            Parser p(std::move(l));
            auto program = p.parseProgram();
            checkParserErrors(p);

            auto actual = program->string();
            REQUIRE(actual == tt.expected);
        }
    }
}

TEST_CASE("Test boolean expression", "[parser]") {
    struct Test {
        std::string input;
        bool expectedBoolean;
    };

    std::vector<Test> tests = {
        {"true;", true},
        {"false;", false},
    };

    for (const auto &tt: tests) {
        Lexer l(tt.input);
        Parser p(std::move(l));
        auto program = p.parseProgram();
        checkParserErrors(p);

        REQUIRE(program->statements.size() == 1);

        auto stmt = dynamic_cast<Ast::ExpressionStatement *>(program->statements[0].get());
        REQUIRE(stmt != nullptr);

        auto boolean = dynamic_cast<Ast::Boolean *>(stmt->expression.get());
        REQUIRE(boolean != nullptr);
        REQUIRE(boolean->value == tt.expectedBoolean);
    }
}

TEST_CASE("Test if expression", "[parser]") {
    std::string input = "if (x < y) { x }";

    Lexer l(input);
    Parser p(std::move(l));
    auto program = p.parseProgram();
    checkParserErrors(p);

    REQUIRE(program->statements.size() == 1);

    auto stmt = dynamic_cast<Ast::ExpressionStatement *>(program->statements[0].get());
    REQUIRE(stmt != nullptr);

    auto exp = dynamic_cast<Ast::IfExpression *>(stmt->expression.get());
    REQUIRE(exp != nullptr);

    // Test condition
    auto condition = dynamic_cast<Ast::InfixExpression *>(exp->condition.get());
    REQUIRE(condition != nullptr);
    testIdentifier(condition->left.get(), "x");
    REQUIRE(condition->operator_ == "<");
    testIdentifier(condition->right.get(), "y");

    // Test consequence
    REQUIRE(exp->consequence->statements.size() == 1);
    auto consequence = dynamic_cast<Ast::ExpressionStatement *>(exp->consequence->statements[0].get());
    REQUIRE(consequence != nullptr);
    testIdentifier(consequence->expression.get(), "x");

    // Test that alternative is null
    REQUIRE(exp->alternative == nullptr);
}

TEST_CASE("Test if-else expression", "[parser]") {
    std::string input = "if (x < y) { x } else { y }";

    Lexer l(input);
    Parser p(std::move(l));
    auto program = p.parseProgram();
    checkParserErrors(p);

    REQUIRE(program->statements.size() == 1);

    auto stmt = dynamic_cast<Ast::ExpressionStatement *>(program->statements[0].get());
    REQUIRE(stmt != nullptr);

    auto exp = dynamic_cast<Ast::IfExpression *>(stmt->expression.get());
    REQUIRE(exp != nullptr);

    // Test condition
    auto condition = dynamic_cast<Ast::InfixExpression *>(exp->condition.get());
    REQUIRE(condition != nullptr);
    testIdentifier(condition->left.get(), "x");
    REQUIRE(condition->operator_ == "<");
    testIdentifier(condition->right.get(), "y");

    // Test consequence
    REQUIRE(exp->consequence->statements.size() == 1);
    auto consequence = dynamic_cast<Ast::ExpressionStatement *>(exp->consequence->statements[0].get());
    REQUIRE(consequence != nullptr);
    testIdentifier(consequence->expression.get(), "x");

    // Test alternative
    REQUIRE(exp->alternative != nullptr);
    REQUIRE(exp->alternative->statements.size() == 1);
    auto alternative = dynamic_cast<Ast::ExpressionStatement *>(exp->alternative->statements[0].get());
    REQUIRE(alternative != nullptr);
    testIdentifier(alternative->expression.get(), "y");
}

TEST_CASE("Test function literal parsing", "[parser]") {
    std::string input = "fn(x, y) { x + y; }";

    Lexer l(input);
    Parser p(std::move(l));
    auto program = p.parseProgram();
    checkParserErrors(p);

    REQUIRE(program->statements.size() == 1);

    auto stmt = dynamic_cast<Ast::ExpressionStatement*>(program->statements[0].get());
    REQUIRE(stmt != nullptr);

    auto function = dynamic_cast<Ast::FunctionLiteral*>(stmt->expression.get());
    REQUIRE(function != nullptr);

    REQUIRE(function->parameters.size() == 2);

    testIdentifier(&function->parameters[0], "x");
    testIdentifier(&function->parameters[1], "y");

    REQUIRE(function->body->statements.size() == 1);

    auto bodyStmt = dynamic_cast<Ast::ExpressionStatement*>(function->body->statements[0].get());
    REQUIRE(bodyStmt != nullptr);

    auto infixExp = dynamic_cast<Ast::InfixExpression*>(bodyStmt->expression.get());
    REQUIRE(infixExp != nullptr);
    testIdentifier(infixExp->left.get(), "x");
    REQUIRE(infixExp->operator_ == "+");
    testIdentifier(infixExp->right.get(), "y");
}

TEST_CASE("Test function parameter parsing", "[parser]") {
    struct Test {
        std::string input;
        std::vector<std::string> expectedParams;
    };

    std::vector<Test> tests = {
        {"fn() {};", {}},
        {"fn(x) {};", {"x"}},
        {"fn(x, y, z) {};", {"x", "y", "z"}}
    };

    for (const auto& tt : tests) {
        SECTION(tt.input) {
            Lexer l(tt.input);
            Parser p(std::move(l));
            auto program = p.parseProgram();
            checkParserErrors(p);

            REQUIRE(program->statements.size() == 1);

            auto stmt = dynamic_cast<Ast::ExpressionStatement*>(program->statements[0].get());
            REQUIRE(stmt != nullptr);

            auto function = dynamic_cast<Ast::FunctionLiteral*>(stmt->expression.get());
            REQUIRE(function != nullptr);

            REQUIRE(function->parameters.size() == tt.expectedParams.size());

            for (size_t i = 0; i < tt.expectedParams.size(); i++) {
                testIdentifier(&function->parameters[i], tt.expectedParams[i]);
            }
        }
    }
}

TEST_CASE("Test call expression parsing", "[parser]") {
    std::string input = "add(1, 2 * 3, 4 + 5);";

    Lexer l(input);
    Parser p(std::move(l));
    auto program = p.parseProgram();
    checkParserErrors(p);

    REQUIRE(program->statements.size() == 1);

    auto stmt = dynamic_cast<Ast::ExpressionStatement*>(program->statements[0].get());
    REQUIRE(stmt != nullptr);

    auto exp = dynamic_cast<Ast::CallExpression*>(stmt->expression.get());
    REQUIRE(exp != nullptr);

    testIdentifier(exp->function.get(), "add");
    REQUIRE(exp->arguments.size() == 3);

    testIntegerLiteral(exp->arguments[0].get(), 1);

    auto infix1 = dynamic_cast<Ast::InfixExpression*>(exp->arguments[1].get());
    REQUIRE(infix1 != nullptr);
    testIntegerLiteral(infix1->left.get(), 2);
    REQUIRE(infix1->operator_ == "*");
    testIntegerLiteral(infix1->right.get(), 3);

    auto infix2 = dynamic_cast<Ast::InfixExpression*>(exp->arguments[2].get());
    REQUIRE(infix2 != nullptr);
    testIntegerLiteral(infix2->left.get(), 4);
    REQUIRE(infix2->operator_ == "+");
    testIntegerLiteral(infix2->right.get(), 5);
}

TEST_CASE("Test call expression parameter parsing", "[parser]") {
    struct Test {
        std::string input;
        std::string expectedIdent;
        std::vector<std::string> expectedArgs;
    };

    std::vector<Test> tests = {
        {
            "add();",
            "add",
            {}
        },
        {
            "add(1);",
            "add",
            {"1"}
        },
        {
            "add(1, 2 * 3, 4 + 5);",
            "add",
            {"1", "(2 * 3)", "(4 + 5)"}
        }
    };

    for (const auto& tt : tests) {
        SECTION(tt.input) {
            Lexer l(tt.input);
            Parser p(std::move(l));
            auto program = p.parseProgram();
            checkParserErrors(p);

            REQUIRE(program->statements.size() == 1);

            auto stmt = dynamic_cast<Ast::ExpressionStatement*>(program->statements[0].get());
            REQUIRE(stmt != nullptr);

            auto exp = dynamic_cast<Ast::CallExpression*>(stmt->expression.get());
            REQUIRE(exp != nullptr);

            testIdentifier(exp->function.get(), tt.expectedIdent);
            REQUIRE(exp->arguments.size() == tt.expectedArgs.size());

            for (size_t i = 0; i < tt.expectedArgs.size(); i++) {
                REQUIRE(exp->arguments[i]->string() == tt.expectedArgs[i]);
            }
        }
    }
}

TEST_CASE("Test string literal expression", "[parser]") {
    const std::string input = "\"hello world\";";

    Lexer l(input);
    Parser p(std::move(l));
    auto program = p.parseProgram();
    checkParserErrors(p);

    auto stmt = dynamic_cast<Ast::ExpressionStatement*>(program->statements[0].get());
    REQUIRE(stmt != nullptr);

    auto literal = dynamic_cast<Ast::StringLiteral*>(stmt->expression.get());
    REQUIRE(literal != nullptr);
    REQUIRE(literal->value == "hello world");
}

TEST_CASE("Test parsing empty array literals", "[parser]") {
    const std::string input = "[]";

    Lexer l(input);
    Parser p(std::move(l));
    auto program = p.parseProgram();
    checkParserErrors(p);

    auto stmt = dynamic_cast<Ast::ExpressionStatement*>(program->statements[0].get());
    REQUIRE(stmt != nullptr);

    auto array = dynamic_cast<Ast::ArrayLiteral*>(stmt->expression.get());
    REQUIRE(array != nullptr);
    REQUIRE(array->elements.size() == 0);
}

TEST_CASE("Test parsing array literals", "[parser]") {
    const std::string input = "[1, 2 * 2, 3 + 3]";

    Lexer l(input);
    Parser p(std::move(l));
    auto program = p.parseProgram();
    checkParserErrors(p);

    auto stmt = dynamic_cast<Ast::ExpressionStatement*>(program->statements[0].get());
    REQUIRE(stmt != nullptr);

    auto array = dynamic_cast<Ast::ArrayLiteral*>(stmt->expression.get());
    REQUIRE(array != nullptr);
    REQUIRE(array->elements.size() == 3);

    testIntegerLiteral(array->elements[0].get(), 1);

    auto infix1 = dynamic_cast<Ast::InfixExpression*>(array->elements[1].get());
    REQUIRE(infix1 != nullptr);
    testIntegerLiteral(infix1->left.get(), 2);
    REQUIRE(infix1->operator_ == "*");
    testIntegerLiteral(infix1->right.get(), 2);

    auto infix2 = dynamic_cast<Ast::InfixExpression*>(array->elements[2].get());
    REQUIRE(infix2 != nullptr);
    testIntegerLiteral(infix2->left.get(), 3);
    REQUIRE(infix2->operator_ == "+");
    testIntegerLiteral(infix2->right.get(), 3);
}

TEST_CASE("Test parsing index expressions", "[parser]") {
    const std::string input = "myArray[1 + 1]";

    Lexer l(input);
    Parser p(std::move(l));
    auto program = p.parseProgram();
    checkParserErrors(p);

    auto stmt = dynamic_cast<Ast::ExpressionStatement*>(program->statements[0].get());
    REQUIRE(stmt != nullptr);

    auto indexExp = dynamic_cast<Ast::IndexExpression*>(stmt->expression.get());
    REQUIRE(indexExp != nullptr);

    testIdentifier(indexExp->left.get(), "myArray");

    auto infix = dynamic_cast<Ast::InfixExpression*>(indexExp->index.get());
    REQUIRE(infix != nullptr);
    testIntegerLiteral(infix->left.get(), 1);
    REQUIRE(infix->operator_ == "+");
    testIntegerLiteral(infix->right.get(), 1);
}

TEST_CASE("Test parsing empty hash literal", "[parser]") {
    const std::string input = "{}";

    Lexer l(input);
    Parser p(std::move(l));
    auto program = p.parseProgram();
    checkParserErrors(p);

    auto stmt = dynamic_cast<Ast::ExpressionStatement*>(program->statements[0].get());
    INFO("Expected ExpressionStatement, got " << typeid(program->statements[0].get()).name());
    REQUIRE(stmt != nullptr);

    auto hash = dynamic_cast<Ast::HashLiteral*>(stmt->expression.get());
    INFO("Expression is not HashLiteral. got=" << typeid(stmt->expression.get()).name());
    REQUIRE(hash != nullptr);

    INFO("hash.pairs has wrong length. got=" << hash->pairs.size());
    REQUIRE(hash->pairs.size() == 0);
}

TEST_CASE("Test parsing hash literals with string keys", "[parser]") {
    const std::string input = "{\"one\": 1, \"two\": 2, \"three\": 3}";

    Lexer l(input);
    Parser p(std::move(l));
    auto program = p.parseProgram();
    checkParserErrors(p);

    auto stmt = dynamic_cast<Ast::ExpressionStatement*>(program->statements[0].get());
    INFO("Expected ExpressionStatement, got " << typeid(program->statements[0].get()).name());
    REQUIRE(stmt != nullptr);

    auto hash = dynamic_cast<Ast::HashLiteral*>(stmt->expression.get());
    INFO("Expression is not HashLiteral. got=" << typeid(stmt->expression.get()).name());
    REQUIRE(hash != nullptr);

    std::map<std::string, int64_t> expected = {
        {"one", 1},
        {"two", 2},
        {"three", 3}
    };

    INFO("hash.pairs has wrong length. expected=" << expected.size() << ", got=" << hash->pairs.size());
    REQUIRE(hash->pairs.size() == expected.size());

    for (const auto& pair : hash->pairs) {
        auto key = dynamic_cast<Ast::StringLiteral*>(pair.first.get());
        INFO("key is not StringLiteral. got=" << typeid(pair.first.get()).name());
        REQUIRE(key != nullptr);

        auto expectedValue = expected[key->value];
        INFO("Testing integer literal for key=" << key->value);
        testIntegerLiteral(pair.second.get(), expectedValue);
    }
}

TEST_CASE("Test parsing hash literals with boolean keys", "[parser]") {
    const std::string input = "{true: 1, false: 2}";

    Lexer l(input);
    Parser p(std::move(l));
    auto program = p.parseProgram();
    checkParserErrors(p);

    auto stmt = dynamic_cast<Ast::ExpressionStatement*>(program->statements[0].get());
    REQUIRE(stmt != nullptr);

    auto hash = dynamic_cast<Ast::HashLiteral*>(stmt->expression.get());
    REQUIRE(hash != nullptr);

    std::map<std::string, int64_t> expected = {
        {"true", 1},
        {"false", 2}
    };

    REQUIRE(hash->pairs.size() == expected.size());

    for (const auto& pair : hash->pairs) {
        auto key = dynamic_cast<Ast::Boolean*>(pair.first.get());
        REQUIRE(key != nullptr);

        auto expectedValue = expected[key->tokenLiteral()];
        testIntegerLiteral(pair.second.get(), expectedValue);
    }
}

TEST_CASE("Test parsing hash literals with integer keys", "[parser]") {
    const std::string input = "{1: 1, 2: 2, 3: 3}";

    Lexer l(input);
    Parser p(std::move(l));
    auto program = p.parseProgram();
    checkParserErrors(p);

    auto stmt = dynamic_cast<Ast::ExpressionStatement*>(program->statements[0].get());
    REQUIRE(stmt != nullptr);

    auto hash = dynamic_cast<Ast::HashLiteral*>(stmt->expression.get());
    REQUIRE(hash != nullptr);

    std::map<std::string, int64_t> expected = {
        {"1", 1},
        {"2", 2},
        {"3", 3}
    };

    REQUIRE(hash->pairs.size() == expected.size());

    for (const auto& pair : hash->pairs) {
        auto key = dynamic_cast<Ast::IntegerLiteral*>(pair.first.get());
        REQUIRE(key != nullptr);

        auto expectedValue = expected[key->tokenLiteral()];
        testIntegerLiteral(pair.second.get(), expectedValue);
    }
}

TEST_CASE("Test parsing hash literals with expressions", "[parser]") {
    const std::string input = "{\"one\": 0 + 1, \"two\": 10 - 8, \"three\": 15 / 5}";

    Lexer l(input);
    Parser p(std::move(l));
    auto program = p.parseProgram();
    checkParserErrors(p);

    auto stmt = dynamic_cast<Ast::ExpressionStatement*>(program->statements[0].get());
    INFO("Expected ExpressionStatement, got " << typeid(program->statements[0].get()).name());
    REQUIRE(stmt != nullptr);

    auto hash = dynamic_cast<Ast::HashLiteral*>(stmt->expression.get());
    INFO("Expression is not HashLiteral. got=" << typeid(stmt->expression.get()).name());
    REQUIRE(hash != nullptr);

    INFO("hash.pairs has wrong length. expected=3, got=" << hash->pairs.size());
    REQUIRE(hash->pairs.size() == 3);

    struct ExpectedExpr {
        int64_t left;
        std::string op;
        int64_t right;
    };

    std::map<std::string, ExpectedExpr> tests = {
        {"one", {0, "+", 1}},
        {"two", {10, "-", 8}},
        {"three", {15, "/", 5}}
    };

    for (const auto& pair : hash->pairs) {
        auto key = dynamic_cast<Ast::StringLiteral*>(pair.first.get());
        INFO("key is not StringLiteral. got=" << typeid(pair.first.get()).name());
        REQUIRE(key != nullptr);

        auto expected = tests[key->value];
        auto exp = dynamic_cast<Ast::InfixExpression*>(pair.second.get());
        INFO("value is not InfixExpression for key=" << key->value << ". got=" << typeid(pair.second.get()).name());
        REQUIRE(exp != nullptr);

        INFO("Testing left value for key=" << key->value);
        testIntegerLiteral(exp->left.get(), expected.left);
        INFO("Testing operator for key=" << key->value << ". expected=" << expected.op << ", got=" << exp->operator_);
        REQUIRE(exp->operator_ == expected.op);
        INFO("Testing right value for key=" << key->value);
        testIntegerLiteral(exp->right.get(), expected.right);
    }
}