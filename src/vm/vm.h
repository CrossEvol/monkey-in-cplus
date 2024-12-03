//
// Created by mizuk on 2024/12/3.
//

#ifndef VM_H
#define VM_H
#include "../object/object.h"
#include "../compiler/compiler.h"
#include "frame.h"

inline constexpr int __stack__size = 2048;
inline constexpr int __globals__size = 65536;
inline constexpr int __max__frames = 1024;

Boolean *nativeBoolToBooleanObject(bool input);

bool isTruthy(Object &object);

class VM {
    std::vector<Object *> constants;

    std::vector<Object *> stack;

    std::vector<Object *> globals;

    std::vector<Frame *> frames;
    int sp;
    int framesIndex;

    void push(Object &object);

    Object *pop();

    void executeBinaryOperation(OpCode op);

    void executeBinaryIntegerOperation(OpCode op, Object &left, Object &right);

    void executeComparison(OpCode op);

    void executeIntegerComparison(OpCode op, Object &left, Object &right);

    void executeBangOperator();

    void executeMinusOperator();

    void executeBinaryStringOperation(OpCode op, Object &left, Object &right);

    Object *buildArray(int startIndex, int endIndex) const;

    Object *buildHash(int startIndex, int endIndex) const;

    void executeIndexExpression(Object &left, Object &index);

    void executeArrayIndex(Object &array, Object &index);

    void executeHashIndex(Object &hash, Object &index);

    Frame *currentFrame() const;

    void pushFrame(Frame &frame);

    Frame *popFrame();

    void executeCall(int numArgs);

    void callClosure(Closure *cl, int numArgs);

    void callBuiltin(const Builtin *builtin, int numArgs);

    void pushClosure(int constIndex, int numFree);

public:
    static Boolean *True;
    static Boolean *False;
    static OBJ::Null *Null;

    explicit VM(const ByteCode &bytecode): sp(0), framesIndex(1) {
        const auto mainFn = new CompiledFunction(bytecode.instructions);
        const auto mainClosure = new Closure(*mainFn);
        const auto mainFrame = new Frame(*mainClosure, 0);

        this->constants = bytecode.constants;
        this->stack = std::vector<Object *>(__stack__size);
        this->globals = std::vector<Object *>(__globals__size);
        this->frames = std::vector<Frame *>(__max__frames);

        this->frames[0] = mainFrame;
    }

    VM(const ByteCode &bytecode, const std::vector<Object *> &s): VM(bytecode) {
        this->globals = s;
    }

    Object *lastPoppedStackElem() const;

    void run();
};

#endif //VM_H
