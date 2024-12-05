//
// Created by mizuk on 2024/11/29.
//

#include "compiler.h"

#include <stdexcept>

#include "fmt/format.h"
#include "../src/common/common.h"

#define USE_TYPE_ID
#define USE_INSTANCE_OF
#undef  USE_INSTANCE_OF

CompilationScope &Compiler::currentScope() const {
    return *this->scopes[this->scopeIndex];
}

int Compiler::addConstant(Object &obj) {
    this->constants.push_back(&obj);
    return this->constants.size() - 1;
}

int Compiler::emit(const OpCode op, const std::vector<int> &operands) {
    const auto ins = Code::make(op, operands);
    const auto pos = this->addInstructions(ins);
    this->setLastInstruction(op, pos);
    return pos;
}

int Compiler::addInstructions(std::vector<std::byte> ins) {
    const auto posNewInstruction = this->currentInstructions().size();
    this->currentInstructions().insert(this->currentInstructions().end(), ins.begin(), ins.end());

    this->currentScope().instructions = this->currentInstructions();
    return posNewInstruction;
}

void Compiler::setLastInstruction(const OpCode op, const int pos) const {
    this->currentScope().previousInstruction = this->currentScope().lastInstruction;
    this->currentScope().lastInstruction = {op, pos};
}

bool Compiler::lastInstructionIs(OpCode op) {
    if (this->currentInstructions().size() == 0) {
        return false;
    }
    return this->currentScope().lastInstruction.opcode == op;
}

void Compiler::removeLastPop() {
    const auto [_, position] = this->currentScope().lastInstruction;
    const auto previous = this->currentScope().previousInstruction;

    auto old = this->currentInstructions();
    const auto newInstructions = std::vector(old.begin(), old.begin() + position);

    this->currentScope().instructions = newInstructions;
    this->currentScope().lastInstruction = previous;
}

void Compiler::replaceInstruction(const int pos, const std::vector<std::byte> &newInstruction) {
    auto &ins = this->currentInstructions();
    for (auto i = 0; i < newInstruction.size(); ++i) {
        ins[pos + i] = newInstruction[i];
    }
}

void Compiler::changeOperand(int opPos, int operand) {
    const auto op = static_cast<OpCode>(this->currentInstructions()[opPos]);
    const auto newInstruction = Code::make(op, {operand});
    this->replaceInstruction(opPos, newInstruction);
}

Instructions &Compiler::currentInstructions() const {
    return this->currentScope().instructions;
}

void Compiler::enterScope() {
    this->scopes.emplace_back<CompilationScope *>(new CompilationScope());
    this->scopeIndex++;

    // Create new symbol table with current one as outer
    auto inner = new SymbolTable(this->symbolTable);
    this->symbolTable = std::make_shared<SymbolTable>(*inner);
}

Instructions Compiler::leaveScope() {
    auto instructions = this->currentInstructions();

    this->scopes.pop_back();
    this->scopeIndex--;
    this->symbolTable = this->symbolTable->outer;

    return instructions;
}

void Compiler::replaceLastPopWithReturn() {
    const int lastPos = this->currentScope().lastInstruction.position;
    this->replaceInstruction(lastPos, Code::make(OpCode::OpReturnValue, {}));
    this->currentScope().lastInstruction.opcode = OpCode::OpReturnValue;
}

void Compiler::loadSymbol(Symbol s) {
    if (s.scope == GlobalScope) {
        this->emit(OpCode::OpGetGlobal, {s.index});
        return;
    }
    if (s.scope == LocalScope) {
        this->emit(OpCode::OpGetLocal, {s.index});
        return;
    }
    if (s.scope == BuiltinScope) {
        this->emit(OpCode::OpGetBuiltin, {s.index});
        return;
    }
    if (s.scope == FreeScope) {
        this->emit(OpCode::OpGetFree, {s.index});
        return;
    }
    return;
}

void Compiler::compile(Ast::Node *_node) {
#ifdef USE_TYPE_ID
    switch (_node->typeID()) {
        case Ast::TypeID::Program_: {
            auto node = dynamic_cast<Ast::Program *>(_node);
            for (auto &s: node->statements) {
                this->compile(s.get());
            }
            break;
        }
        case Ast::TypeID::ExpressionStatement_: {
            auto node = dynamic_cast<Ast::ExpressionStatement *>(_node);
            this->compile(node->expression.get());
            this->emit(OpCode::OpPop, {});
            break;
        }
        case Ast::TypeID::InfixExpression_: {
            auto node = dynamic_cast<Ast::InfixExpression *>(_node);
            if (node->operator_ == "<") {
                this->compile(node->right.get());
                this->compile(node->left.get());
                this->emit(OpCode::OpGreaterThan, {});
                break;
            }

            this->compile(node->left.get());
            this->compile(node->right.get());
            if (node->operator_ == "+") {
                this->emit(OpCode::OpAdd, {});
            } else if (node->operator_ == "-") {
                this->emit(OpCode::OpSub, {});
            } else if (node->operator_ == "*") {
                this->emit(OpCode::OpMul, {});
            } else if (node->operator_ == "/") {
                this->emit(OpCode::OpDiv, {});
            } else if (node->operator_ == ">") {
                this->emit(OpCode::OpGreaterThan, {});
            } else if (node->operator_ == "==") {
                this->emit(OpCode::OpEqual, {});
            } else if (node->operator_ == "!=") {
                this->emit(OpCode::OpNotEqual, {});
            } else {
                throw std::runtime_error(fmt::format("unknown operator {:s}", node->operator_));
            }
            break;
        }
        case Ast::TypeID::IntegerLiteral_: {
            auto node = dynamic_cast<Ast::IntegerLiteral *>(_node);
            auto integer = new Integer(node->value);
            this->emit(OpCode::OpConstant, {this->addConstant(*integer)});
            break;
        }
        case Ast::TypeID::Boolean_: {
            auto node = dynamic_cast<Ast::Boolean *>(_node);
            if (node->value) {
                this->emit(OpCode::OpTrue, {});
            } else {
                this->emit(OpCode::OpFalse, {});
            }
            break;
        }
        case Ast::TypeID::PrefixExpression_: {
            auto node = dynamic_cast<Ast::PrefixExpression *>(_node);
            this->compile(node->right.get());
            if (node->operator_ == "!") {
                this->emit(OpCode::OpBang, {});
            } else if (node->operator_ == "-") {
                this->emit(OpCode::OpMinus, {});
            } else {
                throw std::runtime_error(fmt::format("unknown operator {:s}", node->operator_));
            }
            break;
        }
        case Ast::TypeID::IfExpression_: {
            auto node = dynamic_cast<Ast::IfExpression *>(_node);
            this->compile(node->condition.get());

            // Emit an `OpJumpNotTruthy` with a bogus value
            auto jumpNotTruthyPos = this->emit(OpCode::OpJumpNotTruthy, {9999});

            this->compile(node->consequence.get());

            if (this->lastInstructionIs(OpCode::OpPop)) {
                this->removeLastPop();
            }

            auto jumpPos = this->emit(OpCode::OpJump, {9999});

            auto afterConsequencePos = this->currentInstructions().size();
            this->changeOperand(jumpNotTruthyPos, afterConsequencePos);

            if (node->alternative == nullptr) {
                this->emit(OpCode::OpNull, {});
            } else {
                this->compile(node->alternative.get());

                if (this->lastInstructionIs(OpCode::OpPop)) {
                    this->removeLastPop();
                }
            }

            auto afterAlternativePos = this->currentInstructions().size();
            this->changeOperand(jumpPos, afterAlternativePos);
            break;
        }
        case Ast::TypeID::BlockStatement_: {
            auto node = dynamic_cast<Ast::BlockStatement *>(_node);
            for (auto &s: node->statements) {
                this->compile(s.get());
            }
            break;
        }
        case Ast::TypeID::LetStatement_: {
            auto node = dynamic_cast<Ast::LetStatement *>(_node);
            auto symbol = this->symbolTable->define(node->name->value);
            this->compile(node->value.get());

            if (symbol.scope == GlobalScope) {
                this->emit(OpCode::OpSetGlobal, {symbol.index});
            } else {
                this->emit(OpCode::OpSetLocal, {symbol.index});
            }
            break;
        }
        case Ast::TypeID::Identifier_: {
            auto node = dynamic_cast<Ast::Identifier *>(_node);
            auto [symbol, ok] = this->symbolTable->resolve(node->value);
            if (!ok) {
                throw std::runtime_error(fmt::format("unknown variable {:s}", node->value));
            }

            this->loadSymbol(symbol);
            break;
        }
        case Ast::TypeID::StringLiteral_: {
            auto node = dynamic_cast<Ast::StringLiteral *>(_node);
            auto str = new String(node->value);
            this->emit(OpCode::OpConstant, {this->addConstant(*str)});
            break;
        }
        case Ast::TypeID::ArrayLiteral_: {
            auto node = dynamic_cast<Ast::ArrayLiteral *>(_node);
            for (auto &element: node->elements) {
                this->compile(element.get());
            }
            this->emit(OpCode::OpArray, {static_cast<int>(node->elements.size())});
            break;
        }
        case Ast::TypeID::HashLiteral_: {
            auto node = dynamic_cast<Ast::HashLiteral *>(_node);
            std::vector<Ast::Expression *> keys{};
            for (auto &[_, pair]: node->pairs) {
                keys.push_back(pair.first.get());
            }

            std::sort(keys.begin(), keys.end(), [](Ast::Expression *a, Ast::Expression *b) {
                return a->string() < b->string();
            });

            for (const auto k: keys) {
                this->compile(k);
                this->compile(node->get(*k));
            }

            this->emit(OpCode::OpHash, {static_cast<int>(node->pairs.size() * 2)});
            break;
        }
        case Ast::TypeID::IndexExpression_: {
            auto node = dynamic_cast<Ast::IndexExpression *>(_node);

            this->compile(node->left.get());
            this->compile(node->index.get());

            this->emit(OpCode::OpIndex, {});
            break;
        }
        case Ast::TypeID::FunctionLiteral_: {
            auto node = dynamic_cast<Ast::FunctionLiteral *>(_node);

            this->enterScope();

            for (auto p: node->parameters) {
                this->symbolTable->define(p.value);
            }

            this->compile(node->body.get());

            if (this->lastInstructionIs(OpCode::OpPop)) {
                this->replaceLastPopWithReturn();
            }
            if (!this->lastInstructionIs(OpCode::OpReturnValue)) {
                this->emit(OpCode::OpReturn, {});
            }

            auto free_symbols = this->symbolTable->free_symbols;
            int num_locals = this->symbolTable->num_definitions;
            auto instructions = this->leaveScope();

            for (auto s: free_symbols) {
                this->loadSymbol(s);
            }

            auto compiled_fn = new CompiledFunction(instructions,
                                                    num_locals,
                                                    static_cast<int>(node->parameters.size()));

            auto fn_index = this->addConstant(*compiled_fn);
            this->emit(OpCode::OpClosure, {fn_index, static_cast<int>(free_symbols.size())});

            break;
        }
        case Ast::TypeID::ReturnStatement_: {
            auto node = dynamic_cast<Ast::ReturnStatement *>(_node);

            this->compile(node->returnValue.get());

            this->emit(OpCode::OpReturnValue, {});
            break;
        }
        case Ast::TypeID::CallExpression_: {
            auto node = dynamic_cast<Ast::CallExpression *>(_node);

            this->compile(node->function.get());

            for (auto &a: node->arguments) {
                this->compile(a.get());
            }

            this->emit(OpCode::OpCall, {static_cast<int>(node->arguments.size())});
            break;
        }
        default:
            throw std::runtime_error("unknown node type");
    }
#endif
#ifdef   USE_INSTANCE_OF
    if (instance_of<Ast::Node, Ast::Program>(*_node)) {
        auto node = dynamic_cast<Ast::Program *>(_node);
        for (auto &s: node->statements) {
            this->compile(s.get());
        }
        return;
    }
    if (instance_of<Ast::Node, Ast::ExpressionStatement>(*_node)) {
        auto node = dynamic_cast<Ast::ExpressionStatement *>(_node);
        this->compile(node->expression.get());
        this->emit(OpCode::OpPop, {});
        return;
    }
    if (instance_of<Ast::Node, Ast::InfixExpression>(*_node)) {
        auto node = dynamic_cast<Ast::InfixExpression *>(_node);
        if (node->operator_ == "<") {
            this->compile(node->right.get());
            this->compile(node->left.get());
            this->emit(OpCode::OpGreaterThan, {});
            return;
        }

        this->compile(node->left.get());
        this->compile(node->right.get());
        if (node->operator_ == "+") {
            this->emit(OpCode::OpAdd, {});
        } else if (node->operator_ == "-") {
            this->emit(OpCode::OpSub, {});
        } else if (node->operator_ == "*") {
            this->emit(OpCode::OpMul, {});
        } else if (node->operator_ == "/") {
            this->emit(OpCode::OpDiv, {});
        } else if (node->operator_ == ">") {
            this->emit(OpCode::OpGreaterThan, {});
        } else if (node->operator_ == "==") {
            this->emit(OpCode::OpEqual, {});
        } else if (node->operator_ == "!=") {
            this->emit(OpCode::OpNotEqual, {});
        } else {
            throw std::runtime_error(fmt::format("unknown operator {:s}", node->operator_));
        }
        return;
    }
    if (instance_of<Ast::Node, Ast::IntegerLiteral>(*_node)) {
        auto node = dynamic_cast<Ast::IntegerLiteral *>(_node);
        auto integer = new Integer(node->value);
        this->emit(OpCode::OpConstant, {this->addConstant(*integer)});
        return;
    }
    if (instance_of<Ast::Node, Ast::Boolean>(*_node)) {
        auto node = dynamic_cast<Ast::Boolean *>(_node);
        if (node->value) {
            this->emit(OpCode::OpTrue, {});
        } else {
            this->emit(OpCode::OpFalse, {});
        }
        return;
    }
    if (instance_of<Ast::Node, Ast::PrefixExpression>(*_node)) {
        auto node = dynamic_cast<Ast::PrefixExpression *>(_node);
        this->compile(node->right.get());
        if (node->operator_ == "!") {
            this->emit(OpCode::OpBang, {});
        } else if (node->operator_ == "-") {
            this->emit(OpCode::OpMinus, {});
        } else {
            throw std::runtime_error(fmt::format("unknown operator {:s}", node->operator_));
        }
        return;
    }
    if (instance_of<Ast::Node, Ast::IfExpression>(*_node)) {
        auto node = dynamic_cast<Ast::IfExpression *>(_node);
        this->compile(node->condition.get());

        // Emit an `OpJumpNotTruthy` with a bogus value
        auto jumpNotTruthyPos = this->emit(OpCode::OpJumpNotTruthy, {9999});

        this->compile(node->consequence.get());

        if (this->lastInstructionIs(OpCode::OpPop)) {
            this->removeLastPop();
        }

        auto jumpPos = this->emit(OpCode::OpJump, {9999});

        auto afterConsequencePos = this->currentInstructions().size();
        this->changeOperand(jumpNotTruthyPos, afterConsequencePos);

        if (node->alternative == nullptr) {
            this->emit(OpCode::OpNull, {});
        } else {
            this->compile(node->alternative.get());

            if (this->lastInstructionIs(OpCode::OpPop)) {
                this->removeLastPop();
            }
        }

        auto afterAlternativePos = this->currentInstructions().size();
        this->changeOperand(jumpPos, afterAlternativePos);
        return;
    }
    if (instance_of<Ast::Node, Ast::BlockStatement>(*_node)) {
        auto node = dynamic_cast<Ast::BlockStatement *>(_node);
        for (auto &s: node->statements) {
            this->compile(s.get());
        }
        return;
    }
    if (instance_of<Ast::Node, Ast::LetStatement>(*_node)) {
        auto node = dynamic_cast<Ast::LetStatement *>(_node);
        auto symbol = this->symbolTable->define(node->name->value);
        this->compile(node->value.get());

        if (symbol.scope == GlobalScope) {
            this->emit(OpCode::OpSetGlobal, {symbol.index});
        } else {
            this->emit(OpCode::OpSetLocal, {symbol.index});
        }
        return;
    }
    if (instance_of<Ast::Node, Ast::Identifier>(*_node)) {
        auto node = dynamic_cast<Ast::Identifier *>(_node);
        auto [symbol,ok] = this->symbolTable->resolve(node->value);
        if (!ok) {
            throw std::runtime_error(fmt::format("unknown variable {:s}", node->value));
        }

        this->loadSymbol(symbol);
        return;
    }
    if (instance_of<Ast::Node, Ast::StringLiteral>(*_node)) {
        auto node = dynamic_cast<Ast::StringLiteral *>(_node);
        auto str = new String(node->value);
        this->emit(OpCode::OpConstant, {this->addConstant(*str)});
        return;
    }
    if (instance_of<Ast::Node, Ast::ArrayLiteral>(*_node)) {
        auto node = dynamic_cast<Ast::ArrayLiteral *>(_node);
        for (auto &element: node->elements) {
            this->compile(element.get());
        }
        this->emit(OpCode::OpArray, {static_cast<int>(node->elements.size())});
        return;
    }
    if (instance_of<Ast::Node, Ast::HashLiteral>(*_node)) {
        auto node = dynamic_cast<Ast::HashLiteral *>(_node);
        std::vector<Ast::Expression *> keys{};
        for (auto &[_,pair]: node->pairs) {
            keys.push_back(pair.first.get());
        }

        std::sort(keys.begin(), keys.end(), [](Ast::Expression *a, Ast::Expression *b) {
            return a->string() < b->string();
        });

        for (const auto k: keys) {
            this->compile(k);
            this->compile(node->get(*k));
        }

        this->emit(OpCode::OpHash, {static_cast<int>(node->pairs.size() * 2)});
        return;
    }
    if (instance_of<Ast::Node, Ast::IndexExpression>(*_node)) {
        auto node = dynamic_cast<Ast::IndexExpression *>(_node);

        this->compile(node->left.get());
        this->compile(node->index.get());

        this->emit(OpCode::OpIndex, {});
        return;
    }
    if (instance_of<Ast::Node, Ast::FunctionLiteral>(*_node)) {
        auto node = dynamic_cast<Ast::FunctionLiteral *>(_node);

        this->enterScope();

        for (auto p: node->parameters) {
            this->symbolTable->define(p.value);
        }

        this->compile(node->body.get());

        if (this->lastInstructionIs(OpCode::OpPop)) {
            this->replaceLastPopWithReturn();
        }
        if (!this->lastInstructionIs(OpCode::OpReturnValue)) {
            this->emit(OpCode::OpReturn, {});
        }

        auto free_symbols = this->symbolTable->free_symbols;
        int num_locals = this->symbolTable->num_definitions;
        auto instructions = this->leaveScope();

        for (auto s: free_symbols) {
            this->loadSymbol(s);
        }

        auto compiled_fn = new CompiledFunction(instructions,
            num_locals,
            static_cast<int>(node->parameters.size()));

        auto fn_index = this->addConstant(*compiled_fn);
        this->emit(OpCode::OpClosure, {fn_index, static_cast<int>(free_symbols.size())});

        return;
    }
    if (instance_of<Ast::Node, Ast::ReturnStatement>(*_node)) {
        auto node = dynamic_cast<Ast::ReturnStatement *>(_node);

        this->compile(node->returnValue.get());

        this->emit(OpCode::OpReturnValue, {});
        return;
    }
    if (instance_of<Ast::Node, Ast::CallExpression>(*_node)) {
        auto node = dynamic_cast<Ast::CallExpression *>(_node);

        this->compile(node->function.get());

        for (auto &a: node->arguments) {
            this->compile(a.get());
        }

        this->emit(OpCode::OpCall, {static_cast<int>(node->arguments.size())});
        return;
    }
    return;
#endif
}

ByteCode Compiler::byteCode() const {
    return {this->currentInstructions(), this->constants};
}
