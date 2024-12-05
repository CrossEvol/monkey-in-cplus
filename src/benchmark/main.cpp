//
// Created by mizuk on 2024/12/5.
//

#include <chrono>
#include <iostream>
#include <string>
#include "../lexer/lexer.h"
#include "../parser/parser.h"
#include "../compiler/compiler.h"
#include "../evaluator/evaluator.h"
#include "../object/environment.h"
#include "../vm/vm.h"


// TODO: much more slower than the program written in golang

const std::string input = R"(
let fibonacci = fn(x) {
  if (x == 0) {
    0
  } else {
    if (x == 1) {
      return 1;
    } else {
      fibonacci(x - 1) + fibonacci(x - 2);
    }
  }
};
fibonacci(35);
)";

int main(int argc, char *argv[]) {
    std::string engine = "vm";
    if (argc > 1) {
        engine = argv[1];
    }

    Lexer l(input);
    Parser p(std::move(l));
    auto program = p.parseProgram();

    if (!p.errors().empty()) {
        for (const auto &err: p.errors()) {
            std::cerr << "parser error: " << err << std::endl;
        }
        return 1;
    }

    Object *result;
    std::chrono::duration<double> duration;

    if (engine == "vm") {
        auto comp = std::make_unique<Compiler>();
        try {
            comp->compile(program.get());
        } catch (const std::runtime_error &err) {
            std::cerr << "compiler error: " << err.what() << std::endl;
            return 1;
        }

        auto machine = std::make_unique<VM>(comp->byteCode());
        auto start = std::chrono::high_resolution_clock::now();

        try {
            machine->run();
        } catch (const std::runtime_error &err) {
            std::cerr << "vm error: " << err.what() << std::endl;
            return 1;
        }

        duration = std::chrono::high_resolution_clock::now() - start;
        result = machine->lastPoppedStackElem();
    } else {
        auto env = std::make_shared<Environment>();
        auto start = std::chrono::high_resolution_clock::now();
        auto evaluator = new Evaluator();
        result = evaluator->Eval(*program, *env);
        duration = std::chrono::high_resolution_clock::now() - start;
    }

    std::cout << "engine=" << engine
            << ", result=" << result->inspect()
            << ", duration=" << duration.count() << "s\n";

    return 0;
}
