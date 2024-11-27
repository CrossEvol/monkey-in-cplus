#include <iostream>
#include <catch2/catch_test_macros.hpp>
#include "../src/object/object.h"
#include "../src/object/builtins.h"
#include "../src/object/environment.h"

TEST_CASE("String HashKey", "[object]") {
    auto hello1 = std::make_shared<String>("Hello World");
    auto hello2 = std::make_shared<String>("Hello World");
    auto diff1 = std::make_shared<String>("My name is johnny");
    auto diff2 = std::make_shared<String>("My name is johnny");

    REQUIRE(hello1->hash_key() == hello2->hash_key());
    REQUIRE(diff1->hash_key() == diff2->hash_key());
    REQUIRE(!(hello1->hash_key() == diff1->hash_key()));
}

TEST_CASE("Boolean HashKey", "[object]") {
    auto true1 = std::make_shared<Boolean>(true);
    auto true2 = std::make_shared<Boolean>(true);
    auto false1 = std::make_shared<Boolean>(false);
    auto false2 = std::make_shared<Boolean>(false);

    REQUIRE(true1->hash_key() == true2->hash_key());
    REQUIRE(false1->hash_key() == false2->hash_key());
    REQUIRE(!(true1->hash_key() == false1->hash_key()));
}

TEST_CASE("Integer HashKey", "[object]") {
    auto one1 = std::make_shared<Integer>(1);
    auto one2 = std::make_shared<Integer>(1);
    auto two1 = std::make_shared<Integer>(2);
    auto two2 = std::make_shared<Integer>(2);

    REQUIRE(one1->hash_key() == one2->hash_key());
    REQUIRE(two1->hash_key() == two2->hash_key());
    REQUIRE(!(one1->hash_key() == two1->hash_key()));
}

TEST_CASE("Environment Get and Set", "[environment]") {
    auto env = std::make_shared<Environment>();

    auto obj = std::make_shared<Integer>(5);
    env->set("x", *obj);

    auto [result, ok] = env->get("x");
    REQUIRE(ok);
    REQUIRE(result->type() == INTEGER_OBJ);
    REQUIRE(dynamic_cast<Integer*>(result)->value == 5);

    auto [notFound, notOk] = env->get("y");
    REQUIRE_FALSE(notOk);
}

TEST_CASE("Builtin len function", "[builtins]") {
    SECTION("len with string") {
        std::vector<std::shared_ptr<Object>> args = {std::make_shared<String>("hello")};
        auto result = monkey_len(args);
        REQUIRE(result->type() == INTEGER_OBJ);
        REQUIRE(dynamic_cast<Integer*>(result)->value == 5);
    }

    SECTION("len with array") {
        std::vector<std::shared_ptr<Object>> elements = {
            std::make_shared<Integer>(1),
            std::make_shared<Integer>(2),
            std::make_shared<Integer>(3)
        };
        std::vector<std::shared_ptr<Object>> args = {std::make_shared<Array>(elements)};
        auto result = monkey_len(args);
        REQUIRE(result->type() == INTEGER_OBJ);
        REQUIRE(dynamic_cast<Integer*>(result)->value == 3);
    }

    SECTION("len with wrong number of arguments") {
        std::vector<std::shared_ptr<Object>> args = {};
        auto result = monkey_len(args);
        REQUIRE(result->type() == ERROR_OBJ);
    }

    SECTION("len with unsupported argument") {
        std::vector<std::shared_ptr<Object>> args = {std::make_shared<Integer>(5)};
        auto result = monkey_len(args);
        REQUIRE(result->type() == ERROR_OBJ);
    }
}

TEST_CASE("Builtin first function", "[builtins]") {
    SECTION("first with non-empty array") {
        std::vector<std::shared_ptr<Object>> elements = {
            std::make_shared<Integer>(1),
            std::make_shared<Integer>(2),
            std::make_shared<Integer>(3)
        };
        std::vector<std::shared_ptr<Object>> args = {std::make_shared<Array>(elements)};
        auto result = monkey_first(args);
        REQUIRE(result->type() == INTEGER_OBJ);
        REQUIRE(dynamic_cast<Integer*>(result)->value == 1);
    }

    SECTION("first with empty array") {
        std::vector<std::shared_ptr<Object>> elements = {};
        std::vector<std::shared_ptr<Object>> args = {std::make_shared<Array>(elements)};
        auto result = monkey_first(args);
        REQUIRE(result == nullptr);
    }

    SECTION("first with wrong number of arguments") {
        std::vector<std::shared_ptr<Object>> args = {};
        auto result = monkey_first(args);
        REQUIRE(result->type() == ERROR_OBJ);
    }

    SECTION("first with wrong type of argument") {
        std::vector<std::shared_ptr<Object>> args = {std::make_shared<Integer>(5)};
        auto result = monkey_first(args);
        REQUIRE(result->type() == ERROR_OBJ);
    }
}

TEST_CASE("Builtin last function", "[builtins]") {
    SECTION("last with non-empty array") {
        std::vector<std::shared_ptr<Object>> elements = {
            std::make_shared<Integer>(1),
            std::make_shared<Integer>(2),
            std::make_shared<Integer>(3)
        };
        std::vector<std::shared_ptr<Object>> args = {std::make_shared<Array>(elements)};
        auto result = monkey_last(args);
        REQUIRE(result->type() == INTEGER_OBJ);
        REQUIRE(dynamic_cast<Integer*>(result)->value == 3);
    }

    SECTION("last with empty array") {
        std::vector<std::shared_ptr<Object>> elements = {};
        std::vector<std::shared_ptr<Object>> args = {std::make_shared<Array>(elements)};
        auto result = monkey_last(args);
        REQUIRE(result == nullptr);
    }

    SECTION("last with wrong number of arguments") {
        std::vector<std::shared_ptr<Object>> args = {};
        auto result = monkey_last(args);
        REQUIRE(result->type() == ERROR_OBJ);
    }

    SECTION("last with wrong type of argument") {
        std::vector<std::shared_ptr<Object>> args = {std::make_shared<Integer>(5)};
        auto result = monkey_last(args);
        REQUIRE(result->type() == ERROR_OBJ);
    }
}

TEST_CASE("Builtin rest function", "[builtins]") {
    SECTION("rest with non-empty array") {
        std::vector<std::shared_ptr<Object>> elements = {
            std::make_shared<Integer>(1),
            std::make_shared<Integer>(2),
            std::make_shared<Integer>(3)
        };
        std::vector<std::shared_ptr<Object>> args = {std::make_shared<Array>(elements)};
        auto result = monkey_rest(args);
        REQUIRE(result->type() == ARRAY_OBJ);
        
        auto* array_result = dynamic_cast<Array*>(result);
        REQUIRE(array_result->elements.size() == 2);
        REQUIRE(dynamic_cast<Integer*>(array_result->elements[0].get())->value == 2);
        REQUIRE(dynamic_cast<Integer*>(array_result->elements[1].get())->value == 3);
    }

    SECTION("rest with single element array") {
        std::vector<std::shared_ptr<Object>> elements = {std::make_shared<Integer>(1)};
        std::vector<std::shared_ptr<Object>> args = {std::make_shared<Array>(elements)};
        auto result = monkey_rest(args);
        REQUIRE(result->type() == ARRAY_OBJ);
        
        auto* array_result = dynamic_cast<Array*>(result);
        REQUIRE(array_result->elements.empty());
    }

    SECTION("rest with empty array") {
        std::vector<std::shared_ptr<Object>> elements = {};
        std::vector<std::shared_ptr<Object>> args = {std::make_shared<Array>(elements)};
        auto result = monkey_rest(args);
        REQUIRE(result == nullptr);
    }

    SECTION("rest with wrong number of arguments") {
        std::vector<std::shared_ptr<Object>> args = {};
        auto result = monkey_rest(args);
        REQUIRE(result->type() == ERROR_OBJ);
    }

    SECTION("rest with wrong type of argument") {
        std::vector<std::shared_ptr<Object>> args = {std::make_shared<Integer>(5)};
        auto result = monkey_rest(args);
        REQUIRE(result->type() == ERROR_OBJ);
    }
}

TEST_CASE("Builtin push function", "[builtins]") {
    SECTION("push to non-empty array") {
        std::vector<std::shared_ptr<Object>> elements = {
            std::make_shared<Integer>(1),
            std::make_shared<Integer>(2)
        };
        std::vector<std::shared_ptr<Object>> args = {
            std::make_shared<Array>(elements),
            std::make_shared<Integer>(3)
        };
        auto result = monkey_push(args);
        REQUIRE(result->type() == ARRAY_OBJ);
        
        auto* array_result = dynamic_cast<Array*>(result);
        REQUIRE(array_result->elements.size() == 3);
        REQUIRE(dynamic_cast<Integer*>(array_result->elements[0].get())->value == 1);
        REQUIRE(dynamic_cast<Integer*>(array_result->elements[1].get())->value == 2);
        REQUIRE(dynamic_cast<Integer*>(array_result->elements[2].get())->value == 3);
    }

    SECTION("push to empty array") {
        std::vector<std::shared_ptr<Object>> elements = {};
        std::vector<std::shared_ptr<Object>> args = {
            std::make_shared<Array>(elements),
            std::make_shared<Integer>(1)
        };
        auto result = monkey_push(args);
        REQUIRE(result->type() == ARRAY_OBJ);
        
        auto* array_result = dynamic_cast<Array*>(result);
        REQUIRE(array_result->elements.size() == 1);
        REQUIRE(dynamic_cast<Integer*>(array_result->elements[0].get())->value == 1);
    }

    SECTION("push with wrong number of arguments") {
        std::vector<std::shared_ptr<Object>> args = {std::make_shared<Array>(std::vector<std::shared_ptr<Object>>())};
        auto result = monkey_push(args);
        REQUIRE(result->type() == ERROR_OBJ);
    }

    SECTION("push with wrong type of first argument") {
        std::vector<std::shared_ptr<Object>> args = {
            std::make_shared<Integer>(5),
            std::make_shared<Integer>(1)
        };
        auto result = monkey_push(args);
        REQUIRE(result->type() == ERROR_OBJ);
    }
}
