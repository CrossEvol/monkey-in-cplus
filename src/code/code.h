//
// Created by mizuk on 2024/11/27.
//

#ifndef CODE_H
#define CODE_H
#include <cstddef>
#include <map>
#include <string>
#include <vector>

using Instructions = std::vector<std::byte>;

enum class OpCode:unsigned char {
    OpConstant,

    OpAdd,

    OpPop,

    OpSub,
    OpMul,
    OpDiv,

    OpTrue,
    OpFalse,

    OpEqual,
    OpNotEqual,
    OpGreaterThan,

    OpMinus,
    OpBang,

    OpJumpNotTruthy,
    OpJump,

    OpNull,

    OpGetGlobal,
    OpSetGlobal,

    OpArray,
    OpHash,
    OpIndex,

    OpCall,

    OpReturnValue,
    OpReturn,

    OpGetLocal,
    OpSetLocal,

    OpGetBuiltin,
    OpClosure,

    OpGetFree,
};

struct Definition {
    std::string name;
    std::vector<int> operandWidths;
};


inline std::map<OpCode, Definition> definitions{
    {OpCode::OpConstant, {"OpConstant", std::vector{2}}},

    {OpCode::OpAdd, {"OpAdd", {}}},

    {OpCode::OpPop, {"OpPop", {}}},

    {OpCode::OpSub, {"OpSub", {}}},
    {OpCode::OpMul, {"OpMul", {}}},
    {OpCode::OpDiv, {"OpDiv", {}}},

    {OpCode::OpTrue, {"OpTrue", {}}},
    {OpCode::OpFalse, {"OpFalse", {}}},

    {OpCode::OpEqual, {"OpEqual", {}}},
    {OpCode::OpNotEqual, {"OpNotEqual", {}}},
    {OpCode::OpGreaterThan, {"OpGreaterThan", {}}},

    {OpCode::OpMinus, {"OpMinus", {}}},
    {OpCode::OpBang, {"OpBang", {}}},

    {OpCode::OpJumpNotTruthy, {"OpJumpNotTruthy", {2}}},
    {OpCode::OpJump, {"OpJump", {2}}},

    {OpCode::OpNull, {"OpNull", {}}},

    {OpCode::OpGetGlobal, {"OpGetGlobal", {2}}},
    {OpCode::OpSetGlobal, {"OpSetGlobal", {2}}},

    {OpCode::OpArray, {"OpArray", {2}}},
    {OpCode::OpHash, {"OpHash", {2}}},
    {OpCode::OpIndex, {"OpIndex", {}}},

    {OpCode::OpCall, {"OpCall", {1}}},

    {OpCode::OpReturnValue, {"OpReturnValue", {}}},
    {OpCode::OpReturn, {"OpReturn", {}}},

    {OpCode::OpGetLocal, {"OpGetLocal", {1}}},
    {OpCode::OpSetLocal, {"OpSetLocal", {1}}},

    {OpCode::OpGetBuiltin, {"OpGetBuiltin", {1}}},

    {OpCode::OpClosure, {"OpClosure", {2, 1}}},

    {OpCode::OpGetFree, {"OpGetFree", {1}}},
};

std::string string(Instructions &ins);

std::string fmtInstructions(Definition &def, std::vector<int> operands);

namespace Code {
    Instructions make(OpCode op, const std::vector<int> &operands);
}

std::pair<std::vector<int>, int> readOperands(const Definition &def, const Instructions &ins);

void putUint16BE(Instructions &ins, size_t offset, uint16_t value);

uint8_t readUnit8(const Instructions &ins);

uint16_t readUnit16(const Instructions &ins);

Definition *lookup(uint8_t op);

#endif //CODE_H
