#include <catch2/catch_test_macros.hpp>

#include "../src/code/code.h"
#define CATCH_CONFIG_MAIN

TEST_CASE("test make", "[make]") {
    struct TestCase {
        OpCode op;
        std::vector<int> operands;
        std::vector<std::byte> expected;
    };

    std::vector<TestCase> tests = {
        {
            OpCode::OpConstant,
            {65534},
            {static_cast<std::byte>(static_cast<uint8_t>(OpCode::OpConstant)), std::byte{255}, std::byte{254}}
        },
        {
            OpCode::OpAdd,
            {},
            {static_cast<std::byte>(static_cast<uint8_t>(OpCode::OpAdd))}
        },
        {
            OpCode::OpGetLocal,
            {255},
            {static_cast<std::byte>(static_cast<uint8_t>(OpCode::OpGetLocal)), std::byte{255}}
        },
        {
            OpCode::OpClosure,
            {65534, 255},
            {static_cast<std::byte>(static_cast<uint8_t>(OpCode::OpClosure)), std::byte{255}, std::byte{254}, std::byte{255}}
        }
    };

    for (const auto& tt : tests) {
        Instructions instruction = make(tt.op, tt.operands);

        REQUIRE(instruction.size() == tt.expected.size());

        for (size_t i = 0; i < tt.expected.size(); i++) {
            INFO("Wrong byte at position " << i);
            REQUIRE(instruction[i] == tt.expected[i]);
        }
    }
}

TEST_CASE("test instructions string", "[instructions]") {
    std::vector<Instructions> instructions = {
        make(OpCode::OpAdd, {}),
        make(OpCode::OpGetLocal, {1}),
        make(OpCode::OpConstant, {2}),
        make(OpCode::OpConstant, {65535}),
        make(OpCode::OpClosure, {65535, 255})
    };

    std::string expected = 
        "0000 OpAdd\n"
        "0001 OpGetLocal 1\n"
        "0003 OpConstant 2\n"
        "0006 OpConstant 65535\n"
        "0009 OpClosure 65535 255\n";

    Instructions concatted;
    for (const auto& ins : instructions) {
        concatted.insert(concatted.end(), ins.begin(), ins.end());
    }

    REQUIRE(string(concatted) == expected);
}

TEST_CASE("test read operands", "[operands]") {
    struct TestCase {
        OpCode op;
        std::vector<int> operands;
        int bytesRead;
    };

    std::vector<TestCase> tests = {
        {OpCode::OpConstant, {65535}, 2},
        {OpCode::OpGetLocal, {255}, 1},
        {OpCode::OpClosure, {65535, 255}, 3}
    };

    for (const auto& tt : tests) {
        Instructions instruction = make(tt.op, tt.operands);
        
        Definition* def = lookup(static_cast<uint8_t>(tt.op));
        REQUIRE(def != nullptr);

        Instructions ins_slice(instruction.begin() + 1, instruction.end());
        auto [operandsRead, n] = readOperands(*def, ins_slice);

        REQUIRE(n == tt.bytesRead);

        for (size_t i = 0; i < tt.operands.size(); i++) {
            INFO("Wrong operand at index " << i);
            REQUIRE(operandsRead[i] == tt.operands[i]);
        }
    }
}


