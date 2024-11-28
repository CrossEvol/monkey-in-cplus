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

    Lexer(const Lexer &other)
        : input(other.input),
          position(other.position),
          readPosition(other.readPosition),
          ch(other.ch) {
    }

    Lexer(Lexer &&other) noexcept
        : input(std::move(other.input)),
          position(other.position),
          readPosition(other.readPosition),
          ch(other.ch) {
    }

    Lexer & operator=(const Lexer &other) {
        if (this == &other)
            return *this;
        input = other.input;
        position = other.position;
        readPosition = other.readPosition;
        ch = other.ch;
        return *this;
    }

    Lexer & operator=(Lexer &&other) noexcept {
        if (this == &other)
            return *this;
        input = std::move(other.input);
        position = other.position;
        readPosition = other.readPosition;
        ch = other.ch;
        return *this;
    }

    Token nextToken();
};

#endif //LEXER_H
