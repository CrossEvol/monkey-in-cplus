#include <catch2/catch_test_macros.hpp>
#include <memory>
#include <string>
#include <typeinfo>
#include <variant>

#include "../src/lexer/lexer.h"
#include "../src/parser/parser.h"
#include "../src/evaluator/evaluator.h"
#include "../src/object/object.h"
#include "../src/object/environment.h"
#include "common_suite.h"
#define CATCH_CONFIG_MAIN

namespace EvaluatorTest {
    // Helper function to evaluate input string
    Object *testEval(const std::string &input) {
        Lexer l(input);
        Parser p(std::move(l));
        auto program = p.parseProgram();
        auto env = std::make_unique<Environment>();
        auto evaluator = std::make_unique<Evaluator>();

        return evaluator->Eval(*program, *env);
    }

    // Helper function to test Integer objects
    bool testIntegerObject(Object *obj, int64_t expected) {
        auto result = dynamic_cast<Integer *>(obj);
        if (!result) {
            FAIL("object is not Integer. got=" + std::string(typeid(*obj).name()));
            return false;
        }

        if (result->value != expected) {
            FAIL("object has wrong value. got=" + std::to_string(result->value) +
                ", want=" + std::to_string(expected));
            return false;
        }

        return true;
    }

    // Helper function to test Boolean objects
    bool testBooleanObject(Object *obj, bool expected) {
        auto result = dynamic_cast<Boolean *>(obj);
        if (!result) {
            FAIL("object is not Boolean. got=" + std::string(typeid(*obj).name()));
            return false;
        }

        if (result->value != expected) {
            FAIL("object has wrong value. got=" + std::to_string(result->value) +
                ", want=" + std::to_string(expected));
            return false;
        }

        return true;
    }

    // Helper function to test Null objects
    bool testNullObject(Object *obj) {
        if (obj != Evaluator::Null) {
            FAIL("object is not NULL. got=" + std::string(typeid(*obj).name()));
            return false;
        }
        return true;
    }

    // Helper function to test String objects
    bool testStringObject(Object* obj, const std::string& expected) {
        auto result = dynamic_cast<String*>(obj);
        if (!result) {
            FAIL("object is not String. got=" + std::string(typeid(*obj).name()));
            return false;
        }

        if (result->value != expected) {
            FAIL("String has wrong value. got=" + result->value + 
                 ", want=" + expected);
            return false;
        }

        return true;
    }

    // Helper function to test Array objects
    bool testArrayObject(Object* obj, const std::vector<int64_t>& expected) {
        auto array = dynamic_cast<Array*>(obj);
        if (!array) {
            FAIL("object is not Array. got=" + std::string(typeid(*obj).name()));
            return false;
        }

        if (array->elements.size() != expected.size()) {
            FAIL("wrong num of elements. want=" + std::to_string(expected.size()) + 
                 ", got=" + std::to_string(array->elements.size()));
            return false;
        }

        for (size_t i = 0; i < expected.size(); i++) {
            if (!testIntegerObject(array->elements[i], expected[i])) {
                return false;
            }
        }

        return true;
    }

    // Helper function to test Hash objects
    bool testHashObject(Object* obj, const std::unordered_map<HashKey, int64_t>& expected) {
        auto hash = dynamic_cast<Hash*>(obj);
        if (!hash) {
            FAIL("Eval didn't return Hash. got=" + std::string(typeid(*obj).name()));
            return false;
        }

        if (hash->pairs.size() != expected.size()) {
            FAIL("Hash has wrong num of pairs. got=" + std::to_string(hash->pairs.size()));
            return false;
        }

        for (const auto& [expectedKey, expectedValue] : expected) {
            auto it = hash->pairs.find(expectedKey);
            if (it == hash->pairs.end()) {
                FAIL("no pair for given key in Pairs");
                return false;
            }
            if (!testIntegerObject(it->second.value, expectedValue)) {
                return false;
            }
        }

        return true;
    }

    TEST_CASE("Test integer expression evaluation", "[evaluator]") {
        struct TestCase {
            std::string input;
            int64_t expected;
        };

        std::vector<TestCase> tests = {
            {"5", 5},
            {"10", 10},
            {"-5", -5},
            {"-10", -10},
            {"5 + 5 + 5 + 5 - 10", 10},
            {"2 * 2 * 2 * 2 * 2", 32},
            {"-50 + 100 + -50", 0},
            {"5 * 2 + 10", 20},
            {"5 + 2 * 10", 25},
            {"20 + 2 * -10", 0},
            {"50 / 2 * 2 + 10", 60},
            {"2 * (5 + 10)", 30},
            {"3 * 3 * 3 + 10", 37},
            {"3 * (3 * 3) + 10", 37},
            {"(5 + 10 * 2 + 15 / 3) * 2 + -10", 50},
        };

        for (const auto& tt : tests) {
            Object* evaluated = testEval(tt.input);
            REQUIRE(testIntegerObject(evaluated, tt.expected));
        }
    }

    TEST_CASE("Test boolean expression evaluation", "[evaluator]") {
        struct TestCase {
            std::string input;
            bool expected;
        };

        std::vector<TestCase> tests = {
            {"true", true},
            {"false", false},
            {"1 < 2", true},
            {"1 > 2", false},
            {"1 < 1", false},
            {"1 > 1", false},
            {"1 == 1", true},
            {"1 != 1", false},
            {"1 == 2", false},
            {"1 != 2", true},
            {"true == true", true},
            {"false == false", true},
            {"true == false", false},
            {"true != false", true},
            {"false != true", true},
            {"(1 < 2) == true", true},
            {"(1 < 2) == false", false},
            {"(1 > 2) == true", false},
            {"(1 > 2) == false", true},
        };

        for (const auto& tt : tests) {
            Object* evaluated = testEval(tt.input);
            REQUIRE(testBooleanObject(evaluated, tt.expected));
        }
    }

    TEST_CASE("Test bang operator", "[evaluator]") {
        struct TestCase {
            std::string input;
            bool expected;
        };

        std::vector<TestCase> tests = {
            {"!true", false},
            {"!false", true},
            {"!5", false},
            {"!!true", true},
            {"!!false", false},
            {"!!5", true},
        };

        for (const auto& tt : tests) {
            Object* evaluated = testEval(tt.input);
            REQUIRE(testBooleanObject(evaluated, tt.expected));
        }
    }

     TEST_CASE("Test if-else expressions", "[evaluator]") {
         struct TestCase {
             std::string input;
             std::variant<int64_t, std::nullptr_t> expected;
         };

         std::vector<TestCase> tests = {
             {"if (true) { 10 }", 10},
             {"if (false) { 10 }", nullptr},
             {"if (1) { 10 }", 10},
             {"if (1 < 2) { 10 }", 10},
             {"if (1 > 2) { 10 }", nullptr},
             {"if (1 > 2) { 10 } else { 20 }", 20},
             {"if (1 < 2) { 10 } else { 20 }", 10},
         };

         for (const auto& tt : tests) {
             Object* evaluated = testEval(tt.input);
             if (std::holds_alternative<int64_t>(tt.expected)) {
                 REQUIRE(testIntegerObject(evaluated, std::get<int64_t>(tt.expected)));
             } else {
                 REQUIRE(testNullObject(evaluated));
             }
         }
     }

    TEST_CASE("Test return statements", "[evaluator]") {
        struct TestCase {
            std::string input;
            int64_t expected;
        };

        std::vector<TestCase> tests = {
            {"return 10;", 10},
            {"return 10; 9;", 10},
            {"return 2 * 5; 9;", 10},
            {"9; return 2 * 5; 9;", 10},
            {"if (10 > 1) { return 10; }", 10},
            {R"(
if (10 > 1) {
  if (10 > 1) {
    return 10;
  }
  return 1;
}
)", 10},
            {R"(
let f = fn(x) {
  return x;
  x + 10;
};
f(10);)", 10},
            {R"(
let f = fn(x) {
   let result = x + 10;
   return result;
   return 10;
};
f(10);)", 20},
        };

        for (const auto& tt : tests) {
            Object* evaluated = testEval(tt.input);
            REQUIRE(testIntegerObject(evaluated, tt.expected));
        }
    }

    TEST_CASE("Test error handling", "[evaluator]") {
        struct TestCase {
            std::string input;
            std::string expectedMessage;
        };

        std::vector<TestCase> tests = {
            {"5 + true;", "type mismatch: INTEGER + BOOLEAN"},
            {"5 + true; 5;", "type mismatch: INTEGER + BOOLEAN"},
            {"-true", "unknown operator: -BOOLEAN"},
            {"true + false;", "unknown operator: BOOLEAN + BOOLEAN"},
            {"true + false + true + false;", "unknown operator: BOOLEAN + BOOLEAN"},
            {"5; true + false; 5", "unknown operator: BOOLEAN + BOOLEAN"},
            {R"("Hello" - "World")", "unknown operator: STRING - STRING"},
            {"if (10 > 1) { true + false; }", "unknown operator: BOOLEAN + BOOLEAN"},
            {R"(
if (10 > 1) {
  if (10 > 1) {
    return true + false;
  }
  return 1;
}
)", "unknown operator: BOOLEAN + BOOLEAN"},
            {"foobar", "identifier not found: foobar"},
            {R"({"name": "Monkey"}[fn(x) { x }];)", "unusable as hash key: FUNCTION"},
            {"999[1]", "index operator not supported: INTEGER"},
        };

        for (const auto& tt : tests) {
            Object* evaluated = testEval(tt.input);

            auto errObj = dynamic_cast<Error*>(evaluated);
            REQUIRE(errObj != nullptr);
            REQUIRE(errObj->message == tt.expectedMessage);
        }
    }

    TEST_CASE("Test let statements", "[evaluator]") {
        struct TestCase {
            std::string input;
            int64_t expected;
        };

        std::vector<TestCase> tests = {
            {"let a = 5; a;", 5},
            {"let a = 5 * 5; a;", 25},
            {"let a = 5; let b = a; b;", 5},
            {"let a = 5; let b = a; let c = a + b + 5; c;", 15},
        };

        for (const auto& tt : tests) {
            Object* evaluated = testEval(tt.input);
            REQUIRE(testIntegerObject(evaluated, tt.expected));
        }
    }

    TEST_CASE("Test function object", "[evaluator]") {
        std::string input = "fn(x) { x + 2; };";

        Object* evaluated = testEval(input);
        auto fn = dynamic_cast<Function*>(evaluated);
        
        REQUIRE(fn != nullptr);
        REQUIRE(fn->parameters.size() == 1);
        REQUIRE(fn->parameters[0]->string() == "x");
        
        std::string expectedBody = "(x + 2)";
        REQUIRE(fn->body->string() == expectedBody);
    }

    TEST_CASE("Test function application", "[evaluator]") {
        struct TestCase {
            std::string input;
            int64_t expected;
        };

        std::vector<TestCase> tests = {
            {"let identity = fn(x) { x; }; identity(5);", 5},
            {"let identity = fn(x) { return x; }; identity(5);", 5},
            {"let double = fn(x) { x * 2; }; double(5);", 10},
            {"let add = fn(x, y) { x + y; }; add(5, 5);", 10},
            {"let add = fn(x, y) { x + y; }; add(5 + 5, add(5, 5));", 20},
            {"fn(x) { x; }(5)", 5},
        };

        for (const auto& tt : tests) {
            Object* evaluated = testEval(tt.input);
            REQUIRE(testIntegerObject(evaluated, tt.expected));
        }
    }

    TEST_CASE("Test enclosing environments", "[evaluator]") {
        std::string input = R"(
let first = 10;
let second = 10;
let third = 10;

let ourFunction = fn(first) {
  let second = 20;

  first + second + third;
};

ourFunction(20) + first + second;
)";

        Object* evaluated = testEval(input);
        REQUIRE(testIntegerObject(evaluated, 70));
    }

    TEST_CASE("Test closures", "[evaluator]") {
        std::string input = R"(
let newAdder = fn(x) {
  fn(y) { x + y };
};

let addTwo = newAdder(2);
addTwo(2);
)";

        Object* evaluated = testEval(input);
        REQUIRE(testIntegerObject(evaluated, 4));
    }

    TEST_CASE("Test string literal", "[evaluator]") {
        std::string input = R"("Hello World!")";

        Object* evaluated = testEval(input);
        REQUIRE(testStringObject(evaluated, "Hello World!"));
    }

    TEST_CASE("Test string concatenation", "[evaluator]") {
        std::string input = R"("Hello" + " " + "World!")";

        Object* evaluated = testEval(input);
        REQUIRE(testStringObject(evaluated, "Hello World!"));
    }

    TEST_CASE("Test builtin functions", "[evaluator]") {
        struct TestCase {
            std::string input;
            std::variant<int64_t, std::string, std::nullptr_t, std::vector<int64_t>> expected;
        };

        std::vector<TestCase> tests = {
            {R"(len(""))", int64_t(0)},
            {R"(len("four"))", int64_t(4)},
            {R"(len("hello world"))", int64_t(11)},
            {R"(len(1))", std::string("argument to `len` not supported, got INTEGER")},
            {R"(len("one", "two"))", std::string("wrong number of arguments. got=2, want=1")},
            {"len([1, 2, 3])", int64_t(3)},
            {"len([])", int64_t(0)},
            {R"(puts("hello", "world!"))", nullptr},
            {"first([1, 2, 3])", int64_t(1)},
            {"first([])", nullptr},
            {"first(1)", std::string("argument to `first` must be ARRAY, got INTEGER")},
            {"last([1, 2, 3])", int64_t(3)},
            {"last([])", nullptr},
            {"last(1)", std::string("argument to `last` must be ARRAY, got INTEGER")},
            {"rest([1, 2, 3])", std::vector<int64_t>{2, 3}},
            {"rest([])", nullptr},
            {"push([], 1)", std::vector<int64_t>{1}},
            {"push(1, 1)", std::string("argument to `push` must be ARRAY, got INTEGER")},
        };

        for (const auto& tt : tests) {
            Object* evaluated = testEval(tt.input);
            
            std::visit(overloaded{
                [&](int64_t expected) {
                    REQUIRE(testIntegerObject(evaluated, expected));
                },
                [&](const std::string& expected) {
                    auto error = dynamic_cast<Error*>(evaluated);
                    REQUIRE(error != nullptr);
                    REQUIRE(error->message == expected);
                },
                [&](std::nullptr_t) {
                    REQUIRE(testNullObject(evaluated));
                },
                [&](const std::vector<int64_t>& expected) {
                    REQUIRE(testArrayObject(evaluated, expected));
                }
            }, tt.expected);
        }
    }

    TEST_CASE("Test array literals", "[evaluator]") {
        std::string input = "[1, 2 * 2, 3 + 3]";

        Object* evaluated = testEval(input);
        auto result = dynamic_cast<Array*>(evaluated);
        REQUIRE(result != nullptr);
        REQUIRE(result->elements.size() == 3);

        REQUIRE(testIntegerObject(result->elements[0], 1));
        REQUIRE(testIntegerObject(result->elements[1], 4));
        REQUIRE(testIntegerObject(result->elements[2], 6));
    }

    TEST_CASE("Test array index expressions", "[evaluator]") {
        struct TestCase {
            std::string input;
            std::variant<int64_t, std::nullptr_t> expected;
        };

        std::vector<TestCase> tests = {
            {"[1, 2, 3][0]", int64_t(1)},
            {"[1, 2, 3][1]", int64_t(2)},
            {"[1, 2, 3][2]", int64_t(3)},
            {"let i = 0; [1][i];", int64_t(1)},
            {"[1, 2, 3][1 + 1];", int64_t(3)},
            {"let myArray = [1, 2, 3]; myArray[2];", int64_t(3)},
            {"let myArray = [1, 2, 3]; myArray[0] + myArray[1] + myArray[2];", int64_t(6)},
            {"let myArray = [1, 2, 3]; let i = myArray[0]; myArray[i]", int64_t(2)},
            {"[1, 2, 3][3]", nullptr},
            {"[1, 2, 3][-1]", nullptr},
        };

        for (const auto& tt : tests) {
            Object* evaluated = testEval(tt.input);
            std::visit(overloaded{
                [&](int64_t expected) {
                    REQUIRE(testIntegerObject(evaluated, expected));
                },
                [&](std::nullptr_t) {
                    REQUIRE(testNullObject(evaluated));
                }
            }, tt.expected);
        }
    }

    TEST_CASE("Test hash literals", "[evaluator]") {
        std::string input = R"(
let two = "two";
{
    "one": 10 - 9,
    two: 1 + 1,
    "thr" + "ee": 6 / 2,
    4: 4,
    true: 5,
    false: 6
})";

        std::unordered_map<HashKey, int64_t> expected;
        auto oneStr = std::make_unique<String>("one");
        auto twoStr = std::make_unique<String>("two");
        auto threeStr = std::make_unique<String>("three");
        auto fourInt = std::make_unique<Integer>(4);

        expected[oneStr->hash_key()] = 1;
        expected[twoStr->hash_key()] = 2;
        expected[threeStr->hash_key()] = 3;
        expected[fourInt->hash_key()] = 4;
        expected[Evaluator::True->hash_key()] = 5;
        expected[Evaluator::False->hash_key()] = 6;

        Object* evaluated = testEval(input);
        REQUIRE(testHashObject(evaluated, expected));
    }

    TEST_CASE("Test hash index expressions", "[evaluator]") {
        struct TestCase {
            std::string input;
            std::variant<int64_t, std::nullptr_t> expected;
        };

        std::vector<TestCase> tests = {
            {R"({"foo": 5}["foo"])", int64_t(5)},
            {R"({"foo": 5}["bar"])", nullptr},
            {R"(let key = "foo"; {"foo": 5}[key])", int64_t(5)},
            {R"({}["foo"])", nullptr},
            {"{5: 5}[5]", int64_t(5)},
            {"{true: 5}[true]", int64_t(5)},
            {"{false: 5}[false]", int64_t(5)},
        };

        for (const auto& tt : tests) {
            Object* evaluated = testEval(tt.input);
            std::visit(overloaded{
                [&](int64_t expected) {
                    REQUIRE(testIntegerObject(evaluated, expected));
                },
                [&](std::nullptr_t) {
                    REQUIRE(testNullObject(evaluated));
                }
            }, tt.expected);
        }
    }
}

