//
// Created by mizuk on 2024/11/28.
//

#ifndef LEXER_H
#define LEXER_H

#include <utility>

#include "../token/token.h"

class Lexer {
    std::string input;
    int position;
    int readPosition;
    char ch{};

    void skipWhitespace();

    void readChar();

    [[nodiscard]] char peekChar() const;

    std::string readIdentifier();

    std::string readNumber();

    std::string readString();

    [[nodiscard]] bool isLetter() const;

    [[nodiscard]] bool isDigit() const;


public:
    explicit Lexer(std::string input)
        : input(std::move(input)), position(0), readPosition(0) {
        this->readChar();
    }

    Token nextToken();
};

#endif //LEXER_H
