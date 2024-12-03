//
// Created by mizuk on 2024/12/3.
//

#include "vm.h"

#include <functional>
#include <stdexcept>

#include "../common/common.h"
#include "fmt/format.h"

Boolean *VM::True = new Boolean(true);
Boolean *VM::False = new Boolean(false);
OBJ::Null *VM::Null = new OBJ::Null();

Boolean *nativeBoolToBooleanObject(const bool input) {
    return input ? VM::True : VM::False;
}

bool isTruthy(Object &object) {
    if (instance_of<Object, Boolean>(object)) {
        const auto obj = dynamic_cast<Boolean *>(&object);
        return obj->value;
    }
    if (instance_of<Object, OBJ::Null>(object)) {
        return false;
    }
    return true;
}

void VM::push(Object &object) {
    if (this->sp >= __stack__size) {
        throw std::runtime_error("stack overflow");
    }
    this->stack[this->sp] = &object;
    this->sp++;
}

Object *VM::pop() {
    const auto object = this->stack[this->sp - 1];
    this->sp--;
    return object;
}

void VM::executeBinaryOperation(OpCode op) {
    const auto right = this->pop();
    const auto left = this->pop();

    auto leftType = left->type();
    auto rightType = right->type();

    if (leftType == INTEGER_OBJ && rightType == INTEGER_OBJ) {
        return this->executeBinaryIntegerOperation(op, *left, *right);
    }
    if (leftType == STRING_OBJ && rightType == STRING_OBJ) {
        return this->executeBinaryStringOperation(op, *left, *right);
    }
    throw std::runtime_error(fmt::format("unsupported types for binary operation: {:s} {:s}", leftType, rightType));
}

void VM::executeBinaryIntegerOperation(OpCode op, Object &left, Object &right) {
    const auto leftValue = dynamic_cast<Integer *>(&left)->value;
    const auto rightValue = dynamic_cast<Integer *>(&right)->value;

    int64_t result{};

    switch (op) {
        case OpCode::OpAdd: {
            result = leftValue + rightValue;
            break;
        }
        case OpCode::OpSub: {
            result = leftValue - rightValue;
            break;
        }
        case OpCode::OpMul: {
            result = leftValue * rightValue;
            break;
        }
        case OpCode::OpDiv: {
            result = leftValue / rightValue;
            break;
        }
        default:
            throw std::runtime_error(fmt::format("unknown integer operator: {:d}", static_cast<int>(op)));
    }

    this->push(*new Integer(result));
}

void VM::executeComparison(OpCode op) {
    const auto right = this->pop();
    const auto left = this->pop();

    if (left->type() == INTEGER_OBJ && right->type() == INTEGER_OBJ) {
        this->executeIntegerComparison(op, *left, *right);
        return;
    }

    switch (op) {
        case OpCode::OpEqual: {
            this->push(*nativeBoolToBooleanObject(right == left));
            break;
        }
        case OpCode::OpNotEqual: {
            this->push(*nativeBoolToBooleanObject(right != left));
            break;
        }
        default: {
            throw std::runtime_error(fmt::format("unknown operator: {:d} ({:s} {:s})", static_cast<int>(op),
                                                 left->type(),
                                                 right->type()));
        }
    }
}

void VM::executeIntegerComparison(OpCode op, Object &left, Object &right) {
    const auto leftValue = dynamic_cast<Integer *>(&left)->value;
    const auto rightValue = dynamic_cast<Integer *>(&right)->value;

    switch (op) {
        case OpCode::OpEqual: {
            this->push(*nativeBoolToBooleanObject(rightValue == leftValue));
            break;
        }
        case OpCode::OpNotEqual: {
            this->push(*nativeBoolToBooleanObject(rightValue != leftValue));
            break;
        }
        case OpCode::OpGreaterThan: {
            this->push(*nativeBoolToBooleanObject(leftValue > rightValue));
            break;
        }
        default: {
            throw std::runtime_error(fmt::format("unknown operator: {:d}", static_cast<int>(op)));
        }
    }
}

void VM::executeBangOperator() {
    const auto operand = this->pop();

    if (operand == True) {
        return this->push(*False);
    }
    if (operand == False) {
        return this->push(*True);
    }
    if (operand == Null) {
        return this->push(*True);
    }
    return this->push(*False);
}

void VM::executeMinusOperator() {
    const auto operand = this->pop();

    if (operand->type() != INTEGER_OBJ) {
        throw std::runtime_error(fmt::format("unsupported type for negation: {:s}", operand->type()));
    }

    const auto value = dynamic_cast<Integer *>(operand)->value;
    this->push(*new Integer(-value));
}

void VM::executeBinaryStringOperation(OpCode op, Object &left, Object &right) {
    if (op != OpCode::OpAdd) {
        throw std::runtime_error(fmt::format("unknown string operator: {:d}", static_cast<int>(op)));
    }

    const auto leftValue = dynamic_cast<String *>(&left)->value;
    const auto rightValue = dynamic_cast<String *>(&right)->value;

    this->push(*new String(leftValue + rightValue));
}

Object *VM::buildArray(const int startIndex, const int endIndex) const {
    std::vector<Object *> elements{};
    elements.reserve(endIndex - startIndex);

    for (auto i = startIndex; i < endIndex; i++) {
        elements.push_back(this->stack[i]);
    }

    return new Array(elements);
}

Object *VM::buildHash(const int startIndex, const int endIndex) const {
    std::unordered_map<HashKey, HashPair> hashedPairs{};

    for (auto i = startIndex; i < endIndex; i += 2) {
        const auto key = this->stack[i];
        const auto value = this->stack[i + 1];

        const auto pair = new HashPair(*key, *value);

        const auto hashKey = dynamic_cast<Hashable *>(key);
        if (hashKey == nullptr) {
            throw std::runtime_error(fmt::format("unusable as hash key: {:s}", key->type()));
        }

        hashedPairs[hashKey->hash_key()] = *pair;
    }
    return new Hash(hashedPairs);
}

void VM::executeIndexExpression(Object &left, Object &index) {
    if (left.type() == ARRAY_OBJ && index.type() == INTEGER_OBJ) {
        return this->executeArrayIndex(left, index);
    }
    if (left.type() == HASH_OBJ) {
        return this->executeHashIndex(left, index);
    }
    throw std::runtime_error(fmt::format("index operator not supported: {:s}", left.type()));
}

void VM::executeArrayIndex(Object &array, Object &index) {
    const auto arrayObject = dynamic_cast<Array *>(&array);
    const auto i = dynamic_cast<Integer *>(&index)->value;
    if (i < 0 || i > static_cast<int64_t>(arrayObject->elements.size() - 1)) {
        return this->push(*Null);
    }

    return this->push(*arrayObject->elements[i]);
}

void VM::executeHashIndex(Object &hash, Object &index) {
    const auto hashObject = dynamic_cast<Hash *>(&hash);
    const auto key = dynamic_cast<Hashable *>(&index);
    if (key == nullptr) {
        throw std::runtime_error(fmt::format("unusable as hash key: {:s}", index.type()));
    }

    if (hashObject->pairs.find(key->hash_key()) == hashObject->pairs.end()) {
        return this->push(*Null);
    }
    const auto hash_pair = hashObject->pairs[key->hash_key()];
    return this->push(*hash_pair.value);
}

Frame *VM::currentFrame() const {
    return this->frames[this->framesIndex - 1];
}

void VM::pushFrame(Frame &frame) {
    this->frames[this->framesIndex] = &frame;
    this->framesIndex++;
}

Frame *VM::popFrame() {
    this->framesIndex--;
    return this->frames[this->framesIndex];
}

void VM::executeCall(const int numArgs) {
    const auto callee = this->stack[this->sp - 1 - numArgs];
    if (instance_of<Object, Closure>(*callee)) {
        const auto closure = dynamic_cast<Closure *>(callee);
        return this->callClosure(closure, numArgs);
    }
    if (instance_of<Object, Builtin>(*callee)) {
        const auto builtin = dynamic_cast<Builtin *>(callee);
        return this->callBuiltin(builtin, numArgs);
    }
    throw std::runtime_error("calling non-closure and non-builtin");
}

void VM::callClosure(Closure *cl, int numArgs) {
    if (numArgs != cl->fn.numParameters) {
        throw std::runtime_error(fmt::format("wrong number of arguments: want={:d}, got={:d}",
                                             cl->fn.numParameters, numArgs));
    }

    const auto frame = new Frame(*cl, this->sp - numArgs);
    this->pushFrame(*frame);

    this->sp = frame->basePointer + cl->fn.numLocals;
}

void VM::callBuiltin(const Builtin *builtin, const int numArgs) {
    auto args = std::vector(this->stack.begin() + this->sp - numArgs, this->stack.begin() + this->sp);

    auto result = builtin->fn(args);
    this->sp = this->sp - numArgs - 1;

    if (result != nullptr) {
        this->push(*result);
    } else {
        this->push(*Null);
    }
}

void VM::pushClosure(const int constIndex, const int numFree) {
    const auto constant = this->constants[constIndex];
    const auto function = dynamic_cast<CompiledFunction *>(constant);
    if (function == nullptr) {
        throw std::runtime_error(fmt::format("not a function: {:s}", constant->inspect()));
    }

    std::vector<Object *> free;
    free.reserve(numFree);
    for (auto i = 0; i < numFree; i++) {
        free.push_back(this->stack[this->sp - numFree + i]);
    }
    this->sp = this->sp - numFree;
    this->push(*new Closure(*function, free));
}

Object *VM::lastPoppedStackElem() const {
    return this->stack[this->sp];
}

void VM::run() {
    int ip{0};
    Instructions ins{};
    OpCode op{};

    while (this->currentFrame()->ip < static_cast<int>(this->currentFrame()->instructions().size() - 1)) {
        this->currentFrame()->ip++;

        ip = this->currentFrame()->ip;
        ins = this->currentFrame()->instructions();
        op = static_cast<OpCode>(ins[ip]);

        switch (op) {
            case OpCode::OpConstant: {
                const auto constIndex = readUnit16(std::vector(ins.begin() + ip + 1, ins.end()));
                this->currentFrame()->ip += 2;
                this->push(*this->constants[constIndex]);
                break;
            }
            case OpCode::OpAdd: {
                this->executeBinaryOperation(op);
                break;
            }
            case OpCode::OpPop: {
                this->pop();
                break;
            }
            case OpCode::OpSub: {
                this->executeBinaryOperation(op);
                break;
            }
            case OpCode::OpMul: {
                this->executeBinaryOperation(op);
                break;
            }
            case OpCode::OpDiv: {
                this->executeBinaryOperation(op);
                break;
            }
            case OpCode::OpTrue: {
                this->push(*True);
                break;
            }
            case OpCode::OpFalse: {
                this->push(*False);
                break;
            }
            case OpCode::OpEqual: {
                this->executeComparison(op);
                break;
            }
            case OpCode::OpNotEqual: {
                this->executeComparison(op);
                break;
            }
            case OpCode::OpGreaterThan: {
                this->executeComparison(op);
                break;
            }
            case OpCode::OpMinus: {
                this->executeMinusOperator();
                break;
            }
            case OpCode::OpBang: {
                this->executeBangOperator();
                break;
            }
            case OpCode::OpJumpNotTruthy: {
                const auto pos = readUnit16(std::vector(ins.begin() + ip + 1, ins.end()));
                this->currentFrame()->ip += 2;

                if (const auto condition = this->pop(); !isTruthy(*condition)) {
                    this->currentFrame()->ip = pos - 1;
                }
                break;
            }
            case OpCode::OpJump: {
                const auto pos = readUnit16(std::vector(ins.begin() + ip + 1, ins.end()));
                this->currentFrame()->ip = pos - 1;
                break;
            }
            case OpCode::OpNull: {
                this->push(*Null);
                break;
            }
            case OpCode::OpGetGlobal: {
                const auto globalIndex = readUnit16(std::vector(ins.begin() + ip + 1, ins.end()));
                this->currentFrame()->ip += 2;
                this->push(*this->globals[globalIndex]);
                break;
            }
            case OpCode::OpSetGlobal: {
                const auto globalIndex = readUnit16(std::vector(ins.begin() + ip + 1, ins.end()));
                this->currentFrame()->ip += 2;
                this->globals[globalIndex] = this->pop();
                break;
            }
            case OpCode::OpArray: {
                const auto numElements = readUnit16(std::vector(ins.begin() + ip + 1, ins.end()));
                this->currentFrame()->ip += 2;

                auto array = this->buildArray(this->sp - numElements, this->sp);
                this->sp = this->sp - numElements;
                this->push(*array);
                break;
            }
            case OpCode::OpHash: {
                const auto numElements = readUnit16(std::vector(ins.begin() + ip + 1, ins.end()));
                this->currentFrame()->ip += 2;

                const auto hash = this->buildHash(this->sp - numElements, this->sp);
                this->sp = this->sp - numElements;
                this->push(*hash);
                break;
            }
            case OpCode::OpIndex: {
                const auto index = this->pop();
                const auto left = this->pop();

                this->executeIndexExpression(*left, *index);
                break;
            }
            case OpCode::OpCall: {
                const auto numArgs = readUnit8(std::vector(ins.begin() + ip + 1, ins.end()));
                this->currentFrame()->ip += 1;

                this->executeCall(numArgs);
                break;
            }
            case OpCode::OpReturnValue: {
                const auto returnValue = this->pop();

                const auto frame = this->popFrame();
                this->sp = frame->basePointer - 1;

                this->push(*returnValue);


                break;
            }
            case OpCode::OpReturn: {
                const auto frame = this->popFrame();
                this->sp = frame->basePointer - 1;

                this->push(*Null);
                break;
            }
            case OpCode::OpGetLocal: {
                const auto localIndex = readUnit8(std::vector(ins.begin() + ip + 1, ins.end()));
                this->currentFrame()->ip += 1;

                const auto frame = this->currentFrame();
                this->push(*this->stack[frame->basePointer + localIndex]);
                break;
            }
            case OpCode::OpSetLocal: {
                const auto localIndex = readUnit8(std::vector(ins.begin() + ip + 1, ins.end()));
                this->currentFrame()->ip += 1;

                const auto frame = this->currentFrame();

                this->stack[frame->basePointer + localIndex] = this->pop();
                break;
            }
            case OpCode::OpGetBuiltin: {
                const auto builtinIndex = readUnit8(std::vector(ins.begin() + ip + 1, ins.end()));
                this->currentFrame()->ip += 1;

                auto definition = builtins[builtinIndex];

                this->push(*definition.second);
                break;
            }
            case OpCode::OpClosure: {
                const auto constIndex = readUnit16(std::vector(ins.begin() + ip + 1, ins.end()));
                const auto numFree = readUnit8(std::vector(ins.begin() + ip + 3, ins.end()));
                this->currentFrame()->ip += 3;

                this->pushClosure(constIndex, numFree);
                break;
            }
            case OpCode::OpGetFree: {
                const auto freeIndex = readUnit8(std::vector(ins.begin() + ip + 1, ins.end()));
                this->currentFrame()->ip += 1;

                auto currentClosure = this->currentFrame()->cl;
                this->push(*currentClosure->free[freeIndex]);
                break;
            }
        }
    }
}
