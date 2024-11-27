//
// Created by mizuk on 2024/11/27.
//

#include "code.h"

#include <iostream>
#include <sstream>
#include <stdexcept>
#include <fmt/format.h>

#include "fmt/printf.h"

Definition *lookup(uint8_t op) {
    if (const auto count = definitions.count(static_cast<OpCode>(op)); count > 0) {
        return &definitions.at(static_cast<OpCode>(op));
    }
    throw std::runtime_error(fmt::format("opcode {} undefined", op));
}

void putUint16BE(Instructions &ins, const size_t offset, const uint16_t value) {
    ins[offset] = static_cast<std::byte>(value >> 8 & 0xFF);
    ins[offset + 1] = static_cast<std::byte>(value & 0xFF);
}

Instructions make(OpCode op, const std::vector<int> &operands) {
    const auto iter = definitions.find(op);
    if (iter == definitions.end()) {
        return {};
    }
    const auto &def = iter->second;
    auto instructionLen = 1;
    for (const auto w: def.operandWidths) {
        instructionLen += w;
    }

    Instructions instructions(instructionLen);
    instructions[0] = static_cast<std::byte>(op);

    auto offset = 1;
    for (auto i = 0; i < operands.size(); ++i) {
        const auto width = def.operandWidths[i];
        switch (width) {
            case 1: instructions[offset] = static_cast<std::byte>(operands[i]);
                break;
            case 2: putUint16BE(instructions, offset, operands[i]);
                break;
            default: ;
        }
        offset += width;
    }
    return instructions;
}

uint8_t readUnit8(const Instructions &ins) {
    return static_cast<uint8_t>(ins[0]);
}

uint16_t readUnit16(const Instructions &ins) {
    return (static_cast<uint16_t>(static_cast<uint8_t>(ins[0]) << 8))
           | static_cast<uint16_t>(static_cast<uint8_t>(ins[1]));
}

std::pair<std::vector<int>, int> readOperands(const Definition &def, const Instructions &ins) {
    std::vector<int> operands{};
    auto offset = 0;
    for (const auto width: def.operandWidths) {
        switch (width) {
            case 2: operands.push_back(readUnit16(std::vector(ins.begin() + offset, ins.end())));
                break;
            case 1: operands.push_back(readUnit8(std::vector(ins.begin() + offset, ins.end())));
                break;
            default: ;
        }
        offset += width;
    }
    return {operands, offset};
}


std::string string(Instructions &ins) {
    std::stringstream oss;
    auto i{0};
    while (i < ins.size()) {
        try {
            const auto def = lookup(static_cast<uint8_t>(ins[i]));

            auto [operands,read] = readOperands(*def, std::vector(ins.begin() + i + 1, ins.end()));
            oss << fmt::format("{:04d} {}\n", i, fmtInstructions(*def, operands));
            i += 1 + read;
        } catch (std::runtime_error &e) {
            fmt::print("ERROR: {}\n", e.what());
        }
    }
    return oss.str();
}

std::string fmtInstructions(Definition &def, std::vector<int> operands) {
    auto operandCount = def.operandWidths.size();

    if (operandCount != operands.size()) {
        return fmt::format("ERROR: operand len {} does not match defined {}\n", operands.size(), operandCount);
    }
    switch (operandCount) {
        case 0: return def.name;
        case 1: return fmt::format("{} {}", def.name, operands[0]);
        case 2: return fmt::format("{} {} {}", def.name, operands[0], operands[1]);
        default: ;
    }

    return fmt::format("ERROR: unhandled operandCount for {}\n", def.name);
}
