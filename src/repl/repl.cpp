//
// Created by mizuk on 2024/12/5.
//

#include "repl.h"
#include <string>
#include "../compiler/compiler.h"
#include "../lexer/lexer.h"
#include "../object/object.h"
#include "../parser/parser.h"
#include "../vm/vm.h"

namespace Repl {
    void start(std::istream &in, std::ostream &out) {
        std::vector<Object *> constants;
        const std::vector<Object *> globals(__globals__size);

        const auto symbolTable = new SymbolTable();
        for (size_t i = 0; i < builtins.size(); i++) {
            symbolTable->defineBuiltin(i, builtins[i].first);
        }

        std::string line;
        while (true) {
            out << PROMPT;
            if (!std::getline(in, line)) {
                return;
            }

            const auto lexer = new Lexer(line);
            const auto parser = new Parser(*lexer);

            auto program = parser->parseProgram();
            if (!parser->errors().empty()) {
                printParserErrors(out, parser->errors());
                continue;
            }

            const auto comp = new Compiler(constants, std::make_shared<SymbolTable>(*symbolTable));
            try {
                comp->compile(program.get());
            } catch (std::runtime_error &err) {
                out << "Woops! Compilation failed:\n " << err.what() << "\n";
                continue;
            }

            auto code = comp->byteCode();
            constants = code.constants;

            const auto machine = new VM(code, globals);
            try {
                machine->run();
            } catch (std::runtime_error &err) {
                out << "Woops! Executing bytecode failed:\n " << err.what() << "\n";
                continue;
            }

            const auto lastPopped = machine->lastPoppedStackElem();
            out << lastPopped->inspect() << "\n";
        }
    }

    void printParserErrors(std::ostream &out, const std::vector<std::string> &errors) {
        out << MONKEY_FACE;
        out << "Woops! We ran into some monkey business here!\n";
        out << " parser errors:\n";
        for (const auto &msg: errors) {
            out << "\t" << msg << "\n";
        }
    }
}
