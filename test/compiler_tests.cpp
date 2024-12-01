#include <catch2/catch_test_macros.hpp>
#define CATCH_CONFIG_MAIN

#include <variant>

#include "../src/compiler/compiler.h"
#include "../src/ast/ast.h"
#include "../src/token/token.h"
#include "../src/object/object.h"
#include "../src/code/code.h"
#include "../src/lexer/lexer.h"
#include "../src/parser/parser.h"

using namespace Ast;

Instructions concatInstructions(const std::vector<Instructions> &s);

std::unique_ptr<Program> parse(const std::string &input);

TEST_CASE("TestCompilerScopes", "[compiler]") {
    auto compiler = Compiler();
    REQUIRE(compiler.scopeIndex == 0);
    auto globalSymbolTable = compiler.symbolTable;

    compiler.emit(OpCode::OpMul, {});

    compiler.enterScope();
    REQUIRE(compiler.scopeIndex == 1);

    compiler.emit(OpCode::OpSub, {});

    REQUIRE(compiler.scopes[compiler.scopeIndex]->instructions.size() == 1);

    auto last = compiler.scopes[compiler.scopeIndex]->lastInstruction;
    REQUIRE(last.opcode == OpCode::OpSub);

    REQUIRE(compiler.symbolTable->outer == globalSymbolTable);

    compiler.leaveScope();
    REQUIRE(compiler.scopeIndex == 0);

    REQUIRE(compiler.symbolTable == globalSymbolTable);
    REQUIRE(compiler.symbolTable->outer == nullptr);

    compiler.emit(OpCode::OpAdd, {});

    REQUIRE(compiler.scopes[compiler.scopeIndex]->instructions.size() == 2);

    last = compiler.scopes[compiler.scopeIndex]->lastInstruction;
    REQUIRE(last.opcode == OpCode::OpAdd);

    auto previous = compiler.scopes[compiler.scopeIndex]->previousInstruction;
    REQUIRE(previous.opcode == OpCode::OpMul);
}

struct CompilerTestCase {
    std::string input;
    std::vector<std::variant<int64_t, std::string, std::vector<Instructions> > > expectedConstants;
    std::vector<Instructions> expectedInstructions;
};

void testInstructions(const std::vector<Instructions> &expected, const Instructions &actual) {
    auto concatted = concatInstructions(expected);

    REQUIRE(actual.size() == concatted.size());

    for (size_t i = 0; i < concatted.size(); ++i) {
        REQUIRE(actual[i] == concatted[i]);
    }
}

Instructions concatInstructions(const std::vector<Instructions> &s) {
    Instructions out;
    for (const auto &ins: s) {
        out.insert(out.end(), ins.begin(), ins.end());
    }
    return out;
}

template<class... Ts>
struct overloaded : Ts... {
    using Ts::operator()...;
};

template<class... Ts>
overloaded(Ts...) -> overloaded<Ts...>;

void testConstants(const std::vector<std::variant<int64_t, std::string, std::vector<Instructions> > > &expected,
                   const std::vector<Object *> &actual) {
    REQUIRE(expected.size() == actual.size());

    for (size_t i = 0; i < expected.size(); ++i) {
        std::visit(overloaded{
                       [&](int64_t exp) {
                           auto *integer = dynamic_cast<Integer *>(actual[i]);
                           REQUIRE(integer != nullptr);
                           REQUIRE(integer->value == exp);
                       },
                       [&](const std::string &exp) {
                           auto *str = dynamic_cast<String *>(actual[i]);
                           REQUIRE(str != nullptr);
                           REQUIRE(str->value == exp);
                       },
                       [&](const std::vector<Instructions> &exp) {
                           auto *fn = dynamic_cast<CompiledFunction *>(actual[i]);
                           REQUIRE(fn != nullptr);
                           testInstructions({exp}, fn->instructions);
                       }
                   }, expected[i]);
    }
}

void runCompilerTests(const std::vector<CompilerTestCase> &tests) {
    for (const auto &tt: tests) {
        auto program = parse(tt.input);

        auto compiler = Compiler();
        compiler.compile(program.get());

        auto bytecode = compiler.byteCode();

        testInstructions(tt.expectedInstructions, bytecode.instructions);
        testConstants(tt.expectedConstants, bytecode.constants);
    }
}

std::unique_ptr<Program> parse(const std::string &input) {
    // This function should parse the input string into an AST.
    Lexer l(input);
    Parser p(std::move(l));
    auto program = p.parseProgram();
    return program;
}

TEST_CASE("TestIntegerArithmetic", "[compiler]") {
    std::vector<CompilerTestCase> tests = {
        {
            "1 + 2",
            {1, 2}, // expectedConstants
            {
                Code::make(OpCode::OpConstant, {0}),
                Code::make(OpCode::OpConstant, {1}),
                Code::make(OpCode::OpAdd, {}),
                Code::make(OpCode::OpPop, {})
            }
        },
        {
            "1; 2",
            {1, 2},
            {
                Code::make(OpCode::OpConstant, {0}),
                Code::make(OpCode::OpPop, {}),
                Code::make(OpCode::OpConstant, {1}),
                Code::make(OpCode::OpPop, {})
            }
        },
        {
            "1 - 2",
            {1, 2},
            {
                Code::make(OpCode::OpConstant, {0}),
                Code::make(OpCode::OpConstant, {1}),
                Code::make(OpCode::OpSub, {}),
                Code::make(OpCode::OpPop, {})
            }
        },
        {
            "1 * 2",
            {1, 2},
            {
                Code::make(OpCode::OpConstant, {0}),
                Code::make(OpCode::OpConstant, {1}),
                Code::make(OpCode::OpMul, {}),
                Code::make(OpCode::OpPop, {})
            }
        },
        {
            "2 / 1",
            {2, 1},
            {
                Code::make(OpCode::OpConstant, {0}),
                Code::make(OpCode::OpConstant, {1}),
                Code::make(OpCode::OpDiv, {}),
                Code::make(OpCode::OpPop, {})
            }
        },
        {
            "-1",
            {1},
            {
                Code::make(OpCode::OpConstant, {0}),
                Code::make(OpCode::OpMinus, {}),
                Code::make(OpCode::OpPop, {})
            }
        }
    };

    runCompilerTests(tests);
}

TEST_CASE("TestBooleanExpressions", "[compiler]") {
    std::vector<CompilerTestCase> tests = {
        {
            "true",
            {}, // no constants
            {
                Code::make(OpCode::OpTrue, {}),
                Code::make(OpCode::OpPop, {})
            }
        },
        {
            "false",
            {},
            {
                Code::make(OpCode::OpFalse, {}),
                Code::make(OpCode::OpPop, {})
            }
        },
        {
            "1 > 2",
            {1, 2},
            {
                Code::make(OpCode::OpConstant, {0}),
                Code::make(OpCode::OpConstant, {1}),
                Code::make(OpCode::OpGreaterThan, {}),
                Code::make(OpCode::OpPop, {})
            }
        },
        {
            "1 < 2",
            {2, 1},
            {
                Code::make(OpCode::OpConstant, {0}),
                Code::make(OpCode::OpConstant, {1}),
                Code::make(OpCode::OpGreaterThan, {}),
                Code::make(OpCode::OpPop, {})
            }
        },
        {
            "1 == 2",
            {1, 2},
            {
                Code::make(OpCode::OpConstant, {0}),
                Code::make(OpCode::OpConstant, {1}),
                Code::make(OpCode::OpEqual, {}),
                Code::make(OpCode::OpPop, {})
            }
        },
        {
            "1 != 2",
            {1, 2},
            {
                Code::make(OpCode::OpConstant, {0}),
                Code::make(OpCode::OpConstant, {1}),
                Code::make(OpCode::OpNotEqual, {}),
                Code::make(OpCode::OpPop, {})
            }
        },
        {
            "true == false",
            {},
            {
                Code::make(OpCode::OpTrue, {}),
                Code::make(OpCode::OpFalse, {}),
                Code::make(OpCode::OpEqual, {}),
                Code::make(OpCode::OpPop, {})
            }
        },
        {
            "true != false",
            {},
            {
                Code::make(OpCode::OpTrue, {}),
                Code::make(OpCode::OpFalse, {}),
                Code::make(OpCode::OpNotEqual, {}),
                Code::make(OpCode::OpPop, {})
            }
        },
        {
            "!true",
            {},
            {
                Code::make(OpCode::OpTrue, {}),
                Code::make(OpCode::OpBang, {}),
                Code::make(OpCode::OpPop, {})
            }
        }
    };

    runCompilerTests(tests);
}

TEST_CASE("TestConditionals", "[compiler]") {
    std::vector<CompilerTestCase> tests = {
        {
            "if (true) { 10 }; 3333;",
            {10, 3333},  // expectedConstants
            {
                // 0000
                Code::make(OpCode::OpTrue, {}),
                // 0001
                Code::make(OpCode::OpJumpNotTruthy, {10}),
                // 0004
                Code::make(OpCode::OpConstant, {0}),
                // 0007
                Code::make(OpCode::OpJump, {11}),
                // 0010
                Code::make(OpCode::OpNull, {}),
                // 0011
                Code::make(OpCode::OpPop, {}),
                // 0012
                Code::make(OpCode::OpConstant, {1}),
                // 0015
                Code::make(OpCode::OpPop, {})
            }
        },
        {
            "if (true) { 10 } else { 20 }; 3333;",
            {10, 20, 3333},
            {
                // 0000
                Code::make(OpCode::OpTrue, {}),
                // 0001
                Code::make(OpCode::OpJumpNotTruthy, {10}),
                // 0004
                Code::make(OpCode::OpConstant, {0}),
                // 0007
                Code::make(OpCode::OpJump, {13}),
                // 0010
                Code::make(OpCode::OpConstant, {1}),
                // 0013
                Code::make(OpCode::OpPop, {}),
                // 0014
                Code::make(OpCode::OpConstant, {2}),
                // 0017
                Code::make(OpCode::OpPop, {})
            }
        }
    };

    runCompilerTests(tests);
}

TEST_CASE("TestGlobalLetStatements", "[compiler]") {
    std::vector<CompilerTestCase> tests = {
        {
            "let one = 1;\nlet two = 2;",
            {1, 2},
            {
                Code::make(OpCode::OpConstant, {0}),
                Code::make(OpCode::OpSetGlobal, {0}),
                Code::make(OpCode::OpConstant, {1}),
                Code::make(OpCode::OpSetGlobal, {1})
            }
        },
        {
            "let one = 1;\none;",
            {1},
            {
                Code::make(OpCode::OpConstant, {0}),
                Code::make(OpCode::OpSetGlobal, {0}),
                Code::make(OpCode::OpGetGlobal, {0}),
                Code::make(OpCode::OpPop, {})
            }
        },
        {
            "let one = 1;\nlet two = one;\ntwo;",
            {1},
            {
                Code::make(OpCode::OpConstant, {0}),
                Code::make(OpCode::OpSetGlobal, {0}),
                Code::make(OpCode::OpGetGlobal, {0}),
                Code::make(OpCode::OpSetGlobal, {1}),
                Code::make(OpCode::OpGetGlobal, {1}),
                Code::make(OpCode::OpPop, {})
            }
        }
    };

    runCompilerTests(tests);
}

TEST_CASE("TestFunctions", "[compiler]") {
    std::vector<CompilerTestCase> tests = {
        {
            "fn() { return 5 + 10 }",
            {5, 10, std::vector<Instructions>{
                Code::make(OpCode::OpConstant, {0}),
                Code::make(OpCode::OpConstant, {1}),
                Code::make(OpCode::OpAdd, {}),
                Code::make(OpCode::OpReturnValue, {})
            }},
            {
                Code::make(OpCode::OpClosure, {2, 0}),
                Code::make(OpCode::OpPop, {})
            }
        },
        {
            "fn() { 5 + 10 }",
            {5, 10, std::vector<Instructions>{
                Code::make(OpCode::OpConstant, {0}),
                Code::make(OpCode::OpConstant, {1}),
                Code::make(OpCode::OpAdd, {}),
                Code::make(OpCode::OpReturnValue, {})
            }},
            {
                Code::make(OpCode::OpClosure, {2, 0}),
                Code::make(OpCode::OpPop, {})
            }
        },
        {
            "fn() { 1; 2 }",
            {1, 2, std::vector<Instructions>{
                Code::make(OpCode::OpConstant, {0}),
                Code::make(OpCode::OpPop, {}),
                Code::make(OpCode::OpConstant, {1}),
                Code::make(OpCode::OpReturnValue, {})
            }},
            {
                Code::make(OpCode::OpClosure, {2, 0}),
                Code::make(OpCode::OpPop, {})
            }
        }
    };

    runCompilerTests(tests);
}

TEST_CASE("TestStringExpressions", "[compiler]") {
    std::vector<CompilerTestCase> tests = {
        {
            "\"monkey\"",
            {"monkey"},  // expectedConstants
            {
                Code::make(OpCode::OpConstant, {0}),
                Code::make(OpCode::OpPop, {})
            }
        },
        {
            "\"mon\" + \"key\"",
            {"mon", "key"},
            {
                Code::make(OpCode::OpConstant, {0}),
                Code::make(OpCode::OpConstant, {1}),
                Code::make(OpCode::OpAdd, {}),
                Code::make(OpCode::OpPop, {})
            }
        }
    };

    runCompilerTests(tests);
}

TEST_CASE("TestArrayLiterals", "[compiler]") {
    std::vector<CompilerTestCase> tests = {
        {
            "[]",
            {},  // expectedConstants
            {
                Code::make(OpCode::OpArray, {0}),
                Code::make(OpCode::OpPop, {})
            }
        },
        {
            "[1, 2, 3]",
            {1, 2, 3},
            {
                Code::make(OpCode::OpConstant, {0}),
                Code::make(OpCode::OpConstant, {1}),
                Code::make(OpCode::OpConstant, {2}),
                Code::make(OpCode::OpArray, {3}),
                Code::make(OpCode::OpPop, {})
            }
        },
        {
            "[1 + 2, 3 - 4, 5 * 6]",
            {1, 2, 3, 4, 5, 6},
            {
                Code::make(OpCode::OpConstant, {0}),
                Code::make(OpCode::OpConstant, {1}),
                Code::make(OpCode::OpAdd, {}),
                Code::make(OpCode::OpConstant, {2}),
                Code::make(OpCode::OpConstant, {3}),
                Code::make(OpCode::OpSub, {}),
                Code::make(OpCode::OpConstant, {4}),
                Code::make(OpCode::OpConstant, {5}),
                Code::make(OpCode::OpMul, {}),
                Code::make(OpCode::OpArray, {3}),
                Code::make(OpCode::OpPop, {})
            }
        }
    };

    runCompilerTests(tests);
}

TEST_CASE("TestHashLiterals", "[compiler]") {
    std::vector<CompilerTestCase> tests = {
        {
            "{}",
            {},  // expectedConstants
            {
                Code::make(OpCode::OpHash, {0}),
                Code::make(OpCode::OpPop, {})
            }
        },
        {
            "{1: 2, 3: 4, 5: 6}",
            {1, 2, 3, 4, 5, 6},
            {
                Code::make(OpCode::OpConstant, {0}),
                Code::make(OpCode::OpConstant, {1}),
                Code::make(OpCode::OpConstant, {2}),
                Code::make(OpCode::OpConstant, {3}),
                Code::make(OpCode::OpConstant, {4}),
                Code::make(OpCode::OpConstant, {5}),
                Code::make(OpCode::OpHash, {6}),
                Code::make(OpCode::OpPop, {})
            }
        },
        {
            "{1: 2 + 3, 4: 5 * 6}",
            {1, 2, 3, 4, 5, 6},
            {
                Code::make(OpCode::OpConstant, {0}),
                Code::make(OpCode::OpConstant, {1}),
                Code::make(OpCode::OpConstant, {2}),
                Code::make(OpCode::OpAdd, {}),
                Code::make(OpCode::OpConstant, {3}),
                Code::make(OpCode::OpConstant, {4}),
                Code::make(OpCode::OpConstant, {5}),
                Code::make(OpCode::OpMul, {}),
                Code::make(OpCode::OpHash, {4}),
                Code::make(OpCode::OpPop, {})
            }
        }
    };

    runCompilerTests(tests);
}

TEST_CASE("TestLetStatementScopes", "[compiler]") {
    std::vector<CompilerTestCase> tests = {
        {
            "let num = 55;\nfn() { num }",
            {55, std::vector<Instructions>{
                Code::make(OpCode::OpGetGlobal, {0}),
                Code::make(OpCode::OpReturnValue, {})
            }},
            {
                Code::make(OpCode::OpConstant, {0}),
                Code::make(OpCode::OpSetGlobal, {0}),
                Code::make(OpCode::OpClosure, {1, 0}),
                Code::make(OpCode::OpPop, {})
            }
        },
        {
            "fn() {\n  let num = 55;\n  num\n}",
            {55, std::vector<Instructions>{
                Code::make(OpCode::OpConstant, {0}),
                Code::make(OpCode::OpSetLocal, {0}),
                Code::make(OpCode::OpGetLocal, {0}),
                Code::make(OpCode::OpReturnValue, {})
            }},
            {
                Code::make(OpCode::OpClosure, {1, 0}),
                Code::make(OpCode::OpPop, {})
            }
        },
        {
            "fn() {\n  let a = 55;\n  let b = 77;\n  a + b\n}",
            {55, 77, std::vector<Instructions>{
                Code::make(OpCode::OpConstant, {0}),
                Code::make(OpCode::OpSetLocal, {0}),
                Code::make(OpCode::OpConstant, {1}),
                Code::make(OpCode::OpSetLocal, {1}),
                Code::make(OpCode::OpGetLocal, {0}),
                Code::make(OpCode::OpGetLocal, {1}),
                Code::make(OpCode::OpAdd, {}),
                Code::make(OpCode::OpReturnValue, {})
            }},
            {
                Code::make(OpCode::OpClosure, {2, 0}),
                Code::make(OpCode::OpPop, {})
            }
        }
    };

    runCompilerTests(tests);
}

TEST_CASE("TestBuiltins", "[compiler]") {
    std::vector<CompilerTestCase> tests = {
        {
            "len([]);\npush([], 1);",
            {1},
            {
                Code::make(OpCode::OpGetBuiltin, {0}),
                Code::make(OpCode::OpArray, {0}),
                Code::make(OpCode::OpCall, {1}),
                Code::make(OpCode::OpPop, {}),
                Code::make(OpCode::OpGetBuiltin, {5}),
                Code::make(OpCode::OpArray, {0}),
                Code::make(OpCode::OpConstant, {0}),
                Code::make(OpCode::OpCall, {2}),
                Code::make(OpCode::OpPop, {})
            }
        },
        {
            "fn() { len([]) }",
            {std::vector<Instructions>{
                Code::make(OpCode::OpGetBuiltin, {0}),
                Code::make(OpCode::OpArray, {0}),
                Code::make(OpCode::OpCall, {1}),
                Code::make(OpCode::OpReturnValue, {})
            }},
            {
                Code::make(OpCode::OpClosure, {0, 0}),
                Code::make(OpCode::OpPop, {})
            }
        }
    };

    runCompilerTests(tests);
}

TEST_CASE("TestClosures", "[compiler]") {
    std::vector<CompilerTestCase> tests = {
        {
            "fn(a) {\n  fn(b) {\n    a + b\n  }\n}",
            {std::vector<Instructions>{
                Code::make(OpCode::OpGetFree, {0}),
                Code::make(OpCode::OpGetLocal, {0}),
                Code::make(OpCode::OpAdd, {}),
                Code::make(OpCode::OpReturnValue, {})
            }, std::vector<Instructions>{
                Code::make(OpCode::OpGetLocal, {0}),
                Code::make(OpCode::OpClosure, {0, 1}),
                Code::make(OpCode::OpReturnValue, {})
            }},
            {
                Code::make(OpCode::OpClosure, {1, 0}),
                Code::make(OpCode::OpPop, {})
            }
        },
        {
            "fn(a) {\n  fn(b) {\n    fn(c) {\n      a + b + c\n    }\n  }\n}",
            {std::vector<Instructions>{
                Code::make(OpCode::OpGetFree, {0}),
                Code::make(OpCode::OpGetFree, {1}),
                Code::make(OpCode::OpAdd, {}),
                Code::make(OpCode::OpGetLocal, {0}),
                Code::make(OpCode::OpAdd, {}),
                Code::make(OpCode::OpReturnValue, {})
            }, std::vector<Instructions>{
                Code::make(OpCode::OpGetFree, {0}),
                Code::make(OpCode::OpGetLocal, {0}),
                Code::make(OpCode::OpClosure, {0, 2}),
                Code::make(OpCode::OpReturnValue, {})
            }, std::vector<Instructions>{
                Code::make(OpCode::OpGetLocal, {0}),
                Code::make(OpCode::OpClosure, {1, 1}),
                Code::make(OpCode::OpReturnValue, {})
            }},
            {
                Code::make(OpCode::OpClosure, {2, 0}),
                Code::make(OpCode::OpPop, {})
            }
        },
        {
            "let global = 55;\nfn() {\n  let a = 66;\n  fn() {\n    let b = 77;\n    fn() {\n      let c = 88;\n      global + a + b + c;\n    }\n  }\n}",
            {55, 66, 77, 88, std::vector<Instructions>{
                Code::make(OpCode::OpConstant, {3}),
                Code::make(OpCode::OpSetLocal, {0}),
                Code::make(OpCode::OpGetGlobal, {0}),
                Code::make(OpCode::OpGetFree, {0}),
                Code::make(OpCode::OpAdd, {}),
                Code::make(OpCode::OpGetFree, {1}),
                Code::make(OpCode::OpAdd, {}),
                Code::make(OpCode::OpGetLocal, {0}),
                Code::make(OpCode::OpAdd, {}),
                Code::make(OpCode::OpReturnValue, {})
            }, std::vector<Instructions>{
                Code::make(OpCode::OpConstant, {2}),
                Code::make(OpCode::OpSetLocal, {0}),
                Code::make(OpCode::OpGetFree, {0}),
                Code::make(OpCode::OpGetLocal, {0}),
                Code::make(OpCode::OpClosure, {4, 2}),
                Code::make(OpCode::OpReturnValue, {})
            }, std::vector<Instructions>{
                Code::make(OpCode::OpConstant, {1}),
                Code::make(OpCode::OpSetLocal, {0}),
                Code::make(OpCode::OpGetLocal, {0}),
                Code::make(OpCode::OpClosure, {5, 1}),
                Code::make(OpCode::OpReturnValue, {})
            }},
            {
                Code::make(OpCode::OpConstant, {0}),
                Code::make(OpCode::OpSetGlobal, {0}),
                Code::make(OpCode::OpClosure, {6, 0}),
                Code::make(OpCode::OpPop, {})
            }
        }
    };

    runCompilerTests(tests);
}
