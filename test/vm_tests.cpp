#include <catch2/catch_test_macros.hpp>
#include <variant>
#include <string>
#include <vector>
#include <unordered_map>

#include "common_suite.h"
#include "../cmake-build-debug-mingw/_deps/fmt-src/include/fmt/printf.h"
#include "../src/vm/vm.h"
#include "../src/lexer/lexer.h"
#include "../src/parser/parser.h"
#include "../src/compiler/compiler.h"

namespace VmTest {
    // Helper struct to represent different types of expected values
    struct Expected {
        std::variant<
            int64_t,
            bool,
            std::string,
            std::vector<int>,
            std::unordered_map<HashKey, int64_t>,
            Object *> value;
    };

    struct VMTestCase {
        std::string input;
        Expected expected;
    };

    // Helper function to parse input string to AST
    std::unique_ptr<Ast::Program> parse(const std::string &input) {
        auto lexer = Lexer(input);
        auto parser = Parser(std::move(lexer));
        return parser.parseProgram();
    }

    // Helper functions to test different object types
    std::string testIntegerObject(int64_t expected, Object *actual) {
        auto result = dynamic_cast<Integer *>(actual);
        if (!result) {
            return fmt::format("object is not Integer. got={}", actual->type());
        }

        if (result->value != expected) {
            return fmt::format("object has wrong value. got={}, want={}",
                               result->value, expected);
        }

        return "";
    }

    std::string testBooleanObject(bool expected, Object *actual) {
        auto result = dynamic_cast<Boolean *>(actual);
        if (!result) {
            return fmt::format("object is not Boolean. got={}", actual->type());
        }

        if (result->value != expected) {
            return fmt::format("object has wrong value. got={}, want={}",
                               result->value, expected);
        }

        return "";
    }

    std::string testStringObject(const std::string &expected, Object *actual) {
        auto result = dynamic_cast<String *>(actual);
        if (!result) {
            return fmt::format("object is not String. got={}", actual->type());
        }

        if (result->value != expected) {
            return fmt::format("object has wrong value. got={}, want={}",
                               result->value, expected);
        }

        return "";
    }

    // Helper function to test expected objects against actual VM output
    void testExpectedObject(const Expected &expected, Object *actual) {
        std::visit(overloaded{
                       [&](int64_t exp) {
                           auto err = testIntegerObject(exp, actual);
                           REQUIRE(err.empty());
                       },
                       [&](bool exp) {
                           auto err = testBooleanObject(exp, actual);
                           REQUIRE(err.empty());
                       },
                       [&](const std::string &exp) {
                           auto err = testStringObject(exp, actual);
                           REQUIRE(err.empty());
                       },
                       [&](const std::vector<int> &exp) {
                           auto array = dynamic_cast<Array *>(actual);
                           REQUIRE(array != nullptr);
                           REQUIRE(array->elements.size() == exp.size());

                           for (size_t i = 0; i < exp.size(); i++) {
                               auto err = testIntegerObject(exp[i], array->elements[i]);
                               REQUIRE(err.empty());
                           }
                       },
                       [&](const std::unordered_map<HashKey, int64_t> &exp) {
                           auto hash = dynamic_cast<Hash *>(actual);
                           REQUIRE(hash != nullptr);
                           REQUIRE(hash->pairs.size() == exp.size());

                           for (const auto &[key, value]: exp) {
                               auto it = hash->pairs.find(key);
                               REQUIRE(it != hash->pairs.end());
                               auto err = testIntegerObject(value, it->second.value);
                               REQUIRE(err.empty());
                           }
                       },
                       [&](Object *exp) {
                           if (exp == VM::Null) {
                               REQUIRE(actual == VM::Null);
                           } else if (auto error = dynamic_cast<Error *>(exp)) {
                               auto actualError = dynamic_cast<Error *>(actual);
                               REQUIRE(actualError != nullptr);
                               REQUIRE(actualError->message == error->message);
                           }
                       }
                   }, expected.value);
    }

    // Helper function to run VM tests
    void runVmTests(const std::vector<VMTestCase> &tests) {
        for (const auto &tt: tests) {
            // Parse program
            auto program = parse(tt.input);
            REQUIRE(program != nullptr);

            // Compile program
            auto comp = Compiler();
            try {
                comp.compile(program.get());
            } catch (const std::runtime_error &e) {
                FAIL(fmt::format("compiler error: {}", e.what()));
            }

            // Create and run VM
            auto vm = VM(comp.byteCode());
            try {
                vm.run();
            } catch (const std::runtime_error &e) {
                FAIL(fmt::format("vm error: {}", e.what()));
            }

            auto stackElem = vm.lastPoppedStackElem();
            testExpectedObject(tt.expected, stackElem);
        }
    }

    TEST_CASE("TestIntegerArithmetic") {
        std::vector<VMTestCase> tests = {
            {"1", {1}},
            {"2", {2}},
            {"1 + 2", {3}},
            {"1 - 2", {-1}},
            {"1 * 2", {2}},
            {"4 / 2", {2}},
            {"50 / 2 * 2 + 10 - 5", {55}},
            {"5 * (2 + 10)", {60}},
            {"5 + 5 + 5 + 5 - 10", {10}},
            {"2 * 2 * 2 * 2 * 2", {32}},
            {"5 * 2 + 10", {20}},
            {"5 + 2 * 10", {25}},
            {"5 * (2 + 10)", {60}},
            {"-5", {-5}},
            {"-10", {-10}},
            {"-50 + 100 + -50", {0}},
            {"(5 + 10 * 2 + 15 / 3) * 2 + -10", {50}},
        };

        runVmTests(tests);
    }

    TEST_CASE("TestBooleanExpressions") {
        std::vector<VMTestCase> tests = {
            {"true", {true}},
            {"false", {false}},
            {"1 < 2", {true}},
            {"1 > 2", {false}},
            {"1 < 1", {false}},
            {"1 > 1", {false}},
            {"1 == 1", {true}},
            {"1 != 1", {false}},
            {"1 == 2", {false}},
            {"1 != 2", {true}},
            {"true == true", {true}},
            {"false == false", {true}},
            {"true == false", {false}},
            {"true != false", {true}},
            {"false != true", {true}},
            {"(1 < 2) == true", {true}},
            {"(1 < 2) == false", {false}},
            {"(1 > 2) == true", {false}},
            {"(1 > 2) == false", {true}},
            {"!true", {false}},
            {"!false", {true}},
            {"!5", {false}},
            {"!!true", {true}},
            {"!!false", {false}},
            {"!!5", {true}},
            {"!(if (false) { 5; })", {true}},
        };

        runVmTests(tests);
    }

    TEST_CASE("TestConditionals") {
        std::vector<VMTestCase> tests = {
            {"if (true) { 10 }", {10}},
            {"if (true) { 10 } else { 20 }", {10}},
            {"if (false) { 10 } else { 20 }", {20}},
            {"if (1) { 10 }", {10}},
            {"if (1 < 2) { 10 }", {10}},
            {"if (1 < 2) { 10 } else { 20 }", {10}},
            {"if (1 > 2) { 10 } else { 20 }", {20}},
            {"if (1 > 2) { 10 }", {VM::Null}},
            {"if (false) { 10 }", {VM::Null}},
            {"if ((if (false) { 10 })) { 10 } else { 20 }", {20}},
        };

        runVmTests(tests);
    }

    TEST_CASE("TestGlobalLetStatements") {
        std::vector<VMTestCase> tests = {
            {"let one = 1; one", {1}},
            {"let one = 1; let two = 2; one + two", {3}},
            {"let one = 1; let two = one + one; one + two", {3}},
        };

        runVmTests(tests);
    }

    TEST_CASE("TestStringExpressions") {
        std::vector<VMTestCase> tests = {
            {"\"monkey\"", {"monkey"}},
            {"\"mon\" + \"key\"", {"monkey"}},
            {"\"mon\" + \"key\" + \"banana\"", {"monkeybanana"}},
        };

        runVmTests(tests);
    }

    TEST_CASE("TestArrayLiterals") {
        std::vector<VMTestCase> tests = {
            {"[]", {std::vector<int>{}}},
            {"[1, 2, 3]", {std::vector<int>{1, 2, 3}}},
            {"[1 + 2, 3 * 4, 5 + 6]", {std::vector<int>{3, 12, 11}}},
        };

        runVmTests(tests);
    }

    TEST_CASE("TestHashLiterals") {
        std::vector<VMTestCase> tests = {
            {"{}", {std::unordered_map<HashKey, int64_t>{}}},
            {
                "{1: 2, 2: 3}",
                {std::unordered_map<HashKey, int64_t>{
                    {Integer(1).hash_key(), 2},
                    {Integer(2).hash_key(), 3},
                }}
            },
            {
                "{1 + 1: 2 * 2, 3 + 3: 4 * 4}",
                {std::unordered_map<HashKey, int64_t>{
                    {Integer(2).hash_key(), 4},
                    {Integer(6).hash_key(), 16},
                }}
            },
        };

        runVmTests(tests);
    }

    TEST_CASE("TestIndexExpressions") {
        std::vector<VMTestCase> tests = {
            {"[1, 2, 3][1]", {2}},
            {"[1, 2, 3][0 + 2]", {3}},
            {"[[1, 1, 1]][0][0]", {1}},
            {"[][0]", {VM::Null}},
            {"[1, 2, 3][99]", {VM::Null}},
            {"[1][-1]", {VM::Null}},
            {"{1: 1, 2: 2}[1]", {1}},
            {"{1: 1, 2: 2}[2]", {2}},
            {"{1: 1}[0]", {VM::Null}},
            {"{}[0]", {VM::Null}},
        };

        runVmTests(tests);
    }

    TEST_CASE("TestCallingFunctionsWithoutArguments") {
        std::vector<VMTestCase> tests = {
            {"let fivePlusTen = fn() { 5 + 10; }; fivePlusTen();", {15}},
            {"let one = fn() { 1; }; let two = fn() { 2; }; one() + two()", {3}},
            {"let a = fn() { 1 }; let b = fn() { a() + 1 }; let c = fn() { b() + 1 }; c();", {3}},
        };

        runVmTests(tests);
    }

    TEST_CASE("TestFunctionsWithReturnStatement") {
        std::vector<VMTestCase> tests = {
            {"let earlyExit = fn() { return 99; 100; }; earlyExit();", {99}},
            {"let earlyExit = fn() { return 99; return 100; }; earlyExit();", {99}},
        };

        runVmTests(tests);
    }

    TEST_CASE("TestFunctionsWithoutReturnValue") {
        std::vector<VMTestCase> tests = {
            {"let noReturn = fn() { }; noReturn();", {VM::Null}},
            {"let noReturn = fn() { }; let noReturnTwo = fn() { noReturn(); }; noReturn(); noReturnTwo();", {VM::Null}},
        };

        runVmTests(tests);
    }

    TEST_CASE("TestFirstClassFunctions") {
        std::vector<VMTestCase> tests = {
            {
                "let returnsOne = fn() { 1; }; "
                "let returnsOneReturner = fn() { returnsOne; }; "
                "returnsOneReturner()();",
                {1}
            },
            {
                "let returnsOneReturner = fn() { "
                "    let returnsOne = fn() { 1; }; "
                "    returnsOne; "
                "}; "
                "returnsOneReturner()();",
                {1}
            },
        };

        runVmTests(tests);
    }

    TEST_CASE("TestCallingFunctionsWithBindings") {
        std::vector<VMTestCase> tests = {
            {
                "let one = fn() { let one = 1; one }; "
                "one();",
                {1}
            },
            {
                "let oneAndTwo = fn() { let one = 1; let two = 2; one + two; }; "
                "oneAndTwo();",
                {3}
            },
            {
                "let oneAndTwo = fn() { let one = 1; let two = 2; one + two; }; "
                "let threeAndFour = fn() { let three = 3; let four = 4; three + four; }; "
                "oneAndTwo() + threeAndFour();",
                {10}
            },
            {
                "let firstFoobar = fn() { let foobar = 50; foobar; }; "
                "let secondFoobar = fn() { let foobar = 100; foobar; }; "
                "firstFoobar() + secondFoobar();",
                {150}
            },
            {
                "let globalSeed = 50; "
                "let minusOne = fn() { "
                "    let num = 1; "
                "    globalSeed - num; "
                "}; "
                "let minusTwo = fn() { "
                "    let num = 2; "
                "    globalSeed - num; "
                "}; "
                "minusOne() + minusTwo();",
                {97}
            },
        };

        runVmTests(tests);
    }

    TEST_CASE("TestCallingFunctionsWithArgumentsAndBindings") {
        std::vector<VMTestCase> tests = {
            {
                "let identity = fn(a) { a; }; "
                "identity(4);",
                {4}
            },
            {
                "let sum = fn(a, b) { a + b; }; "
                "sum(1, 2);",
                {3}
            },
            {
                "let sum = fn(a, b) { "
                "    let c = a + b; "
                "    c; "
                "}; "
                "sum(1, 2);",
                {3}
            },
            {
                "let sum = fn(a, b) { "
                "    let c = a + b; "
                "    c; "
                "}; "
                "sum(1, 2) + sum(3, 4);",
                {10}
            },
            {
                "let sum = fn(a, b) { "
                "    let c = a + b; "
                "    c; "
                "}; "
                "let outer = fn() { "
                "    sum(1, 2) + sum(3, 4); "
                "}; "
                "outer();",
                {10}
            },
            {
                "let globalNum = 10; "
                "let sum = fn(a, b) { "
                "    let c = a + b; "
                "    c + globalNum; "
                "}; "
                "let outer = fn() { "
                "    sum(1, 2) + sum(3, 4) + globalNum; "
                "}; "
                "outer() + globalNum;",
                {50}
            },
        };

        runVmTests(tests);
    }

    TEST_CASE("TestCallingFunctionsWithWrongArguments") {
        std::vector<VMTestCase> tests = {
            {"fn() { 1; }(1);", {new Error("wrong number of arguments: want=0, got=1")}},
            {"fn(a) { a; }();", {new Error("wrong number of arguments: want=1, got=0")}},
            {"fn(a, b) { a + b; }(1);", {new Error("wrong number of arguments: want=2, got=1")}},
        };

        for (const auto& tt : tests) {
            // Parse program
            auto program = parse(tt.input);
            REQUIRE(program != nullptr);

            // Compile program
            auto comp = Compiler();
            try {
                comp.compile(program.get());
            } catch (const std::runtime_error& e) {
                FAIL(fmt::format("compiler error: {}", e.what()));
            }

            // Create and run VM
            auto vm = VM(comp.byteCode());
            try {
                vm.run();
                FAIL("expected VM error but got none");
            } catch (const std::runtime_error& e) {
                auto expectedError = dynamic_cast<Error*>(std::get<Object*>(tt.expected.value));
                REQUIRE(e.what() == expectedError->message);
            }
        }
    }

    TEST_CASE("TestBuiltinFunctions") {
        std::vector<VMTestCase> tests = {
            {R"(len(""))", {0}},
            {R"(len("four"))", {4}},
            {R"(len("hello world"))", {11}},
            {"len(1)", {new Error("argument to `len` not supported, got INTEGER")}},
            {"len(\"one\", \"two\")", {new Error("wrong number of arguments. got=2, want=1")}},
            {"len([1, 2, 3])", {3}},
            {"len([])", {0}},
            {"puts(\"hello\", \"world!\")", {VM::Null}},
            {"first([1, 2, 3])", {1}},
            {"first([])", {VM::Null}},
            {"first(1)", {new Error("argument to `first` must be ARRAY, got INTEGER")}},
            {"last([1, 2, 3])", {3}},
            {"last([])", {VM::Null}},
            {"last(1)", {new Error("argument to `last` must be ARRAY, got INTEGER")}},
            {"rest([1, 2, 3])", {std::vector<int>{2, 3}}},
            {"rest([])", {VM::Null}},
            {"push([], 1)", {std::vector<int>{1}}},
            {"push(1, 1)", {new Error("argument to `push` must be ARRAY, got INTEGER")}},
        };

        runVmTests(tests);
    }

    TEST_CASE("TestClosures") {
        std::vector<VMTestCase> tests = {
            {
                "let newClosure = fn(a) { "
                "    fn() { a; }; "
                "}; "
                "let closure = newClosure(99); "
                "closure();",
                {99}
            },
            {
                "let newAdder = fn(a, b) { "
                "    fn(c) { a + b + c }; "
                "}; "
                "let adder = newAdder(1, 2); "
                "adder(8);",
                {11}
            },
            {
                "let newAdder = fn(a, b) { "
                "    let c = a + b; "
                "    fn(d) { c + d }; "
                "}; "
                "let adder = newAdder(1, 2); "
                "adder(8);",
                {11}
            },
            {
                "let newAdderOuter = fn(a, b) { "
                "    let c = a + b; "
                "    fn(d) { "
                "        let e = d + c; "
                "        fn(f) { e + f; }; "
                "    }; "
                "}; "
                "let newAdderInner = newAdderOuter(1, 2); "
                "let adder = newAdderInner(3); "
                "adder(8);",
                {14}
            },
            {
                "let a = 1; "
                "let newAdderOuter = fn(b) { "
                "    fn(c) { "
                "        fn(d) { a + b + c + d }; "
                "    }; "
                "}; "
                "let newAdderInner = newAdderOuter(2); "
                "let adder = newAdderInner(3); "
                "adder(8);",
                {14}
            },
            {
                "let newClosure = fn(a, b) { "
                "    let one = fn() { a; }; "
                "    let two = fn() { b; }; "
                "    fn() { one() + two(); }; "
                "}; "
                "let closure = newClosure(9, 90); "
                "closure();",
                {99}
            },
        };

        runVmTests(tests);
    }

    TEST_CASE("TestRecursiveFibonacci") {
        std::vector<VMTestCase> tests = {
            {
                "let fibonacci = fn(x) { "
                "    if (x == 0) { "
                "        return 0; "
                "    } else { "
                "        if (x == 1) { "
                "            return 1; "
                "        } else { "
                "            fibonacci(x - 1) + fibonacci(x - 2); "
                "        } "
                "    } "
                "}; "
                "fibonacci(15);",
                {610}
            },
        };

        runVmTests(tests);
    }
}
