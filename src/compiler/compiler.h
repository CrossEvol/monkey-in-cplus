//
// Created by mizuk on 2024/11/29.
//

#ifndef COMPILER_H
#define COMPILER_H

#include "../ast/ast.h"
#include "../code/code.h"
#include "../object/object.h"
#include "./symbol_table.h"
#include "../object/builtins.h"


struct ByteCode {
    Instructions instructions{};
    std::vector<Object *> constants{};
};

struct EmittedInstructions {
    OpCode opcode;
    int position;
};

struct CompilationScope {
    Instructions instructions{};
    EmittedInstructions lastInstruction{};
    EmittedInstructions previousInstruction{};
};

class Compiler {
    // TODO: for tests, in the golang we have package visibility, but in c++, we have class visibility
public:
    std::vector<Object *> constants{};
    std::shared_ptr<SymbolTable> symbolTable;
    std::vector<CompilationScope *> scopes{};
    int scopeIndex{0};

    void defineBuiltins() const {
        auto i = 0;
        for (auto [fst, snd]: builtins) {
            this->symbolTable->defineBuiltin(i, fst);
            i++;
        }
    }

    CompilationScope &currentScope() const;

    int addConstant(Object &obj);

    int emit(OpCode op, const std::vector<int> &operands);

    int addInstructions(std::vector<std::byte> ins);

    void setLastInstruction(OpCode op, int pos) const;

    bool lastInstructionIs(OpCode op);

    void removeLastPop();

    void replaceInstruction(int pos, const std::vector<std::byte> &newInstruction);

    void changeOperand(int opPos, int operand);

    Instructions &currentInstructions() const;

    void enterScope();

    Instructions leaveScope();

    void replaceLastPopWithReturn();

    void loadSymbol(Symbol s);

//======================================================================= package level export

public:
    Compiler(): symbolTable{new SymbolTable()} {
        this->scopes.emplace_back(new CompilationScope());
        this->defineBuiltins();
    }

    Compiler(const std::vector<Object *> &constants, const std::shared_ptr<SymbolTable> &symbol_table)
        : Compiler{} {
        this->constants = std::move(constants);

        this->symbolTable = symbol_table;
    }

    void compile(Ast::Node *_node);

    ByteCode byteCode() const;
};

#endif //COMPILER_H
