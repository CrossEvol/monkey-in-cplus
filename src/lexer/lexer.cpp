//
// Created by mizuk on 2024/11/28.
//

#include "lexer.h"

#include <vector>

void Lexer::skipWhitespace() {
    while (this->ch == ' '
           || this->ch == '\t'
           || this->ch == '\n'
           || this->ch == '\r') {
        this->readChar();
    }
}

void Lexer::readChar() {
    if (this->readPosition >= this->input.length()) {
        this->ch = 0;
    } else {
        this->ch = this->input[this->readPosition];
    }
    this->position = this->readPosition;
    this->readPosition++;
}

char Lexer::peekChar() const {
    if (this->readPosition >= this->input.length()) {
        return 0;
    }
    return this->input[this->readPosition];
}

std::string Lexer::readIdentifier() {
    const auto position = this->position;
    while (this->isLetter()) {
        this->readChar();
    }
    return {this->input.begin() + position, this->input.begin() + this->position};
}

std::string Lexer::readNumber() {
    const auto position = this->position;
    while (this->isDigit()) {
        this->readChar();
    }
    return {this->input.begin() + position, this->input.begin() + this->position};
}

std::string Lexer::readString() {
    const auto position = this->position + 1;
    for (;;) {
        this->readChar();
        if (this->ch == '"' || this->ch == 0) {
            break;
        }
    }
    return {this->input.begin() + position, this->input.begin() + this->position};
}

bool Lexer::isLetter() const {
    return 'a' <= this->ch && this->ch <= 'z'
           || 'A' <= this->ch && this->ch <= 'Z'
           || this->ch == '_';
}

bool Lexer::isDigit() const {
    return '0' <= ch && ch <= '9';
}

Token Lexer::nextToken() {
    Token token{};

    this->skipWhitespace();

    switch (this->ch) {
        case '=':
            if (this->peekChar() == '=') {
                auto prev = this->ch;
                this->readChar();
                token = {EQ, std::string(1, prev) + std::string(1, this->ch)};
            } else {
                token = {ASSIGN, this->ch};
            }
            break;
        case '+':
            token = {PLUS, this->ch};
            break;
        case '-':
            token = {MINUS, this->ch};
            break;
        case '!':
            if (this->peekChar() == '=') {
                auto prev = this->ch;
                this->readChar();
                token = {NOT_EQ, std::string(1, prev) + std::string(1, this->ch)};
            } else {
                token = {BANG, this->ch};
            }
            break;
        case '/':
            token = {SLASH, this->ch};
            break;
        case '*':
            token = {ASTERISK, this->ch};
            break;
        case '<':
            token = {LT, this->ch};
            break;
        case '>':
            token = {GT, this->ch};
            break;
        case ';':
            token = {SEMICOLON, this->ch};
            break;
        case ':':
            token = {COLON, this->ch};
            break;
        case ',':
            token = {COMMA, this->ch};
            break;
        case '{':
            token = {LBRACE, this->ch};
            break;
        case '}':
            token = {RBRACE, this->ch};
            break;
        case '(':
            token = {LPAREN, this->ch};
            break;
        case ')':
            token = {RPAREN, this->ch};
            break;
        case '"':
            token = {STRING, this->readString()};
            break;
        case '[':
            token = {LBRACKET, this->ch};
            break;
        case ']':
            token = {RBRACKET, this->ch};
            break;
        case 0:
            token = {EOF_, ""};
            break;
        default: {
            if (isLetter()) {
                auto ident = this->readIdentifier();
                return {lookupIdent(ident), ident};
            }
            if (isDigit()) {
                return {INT, this->readNumber()};
            }
            token = {ILLEGAL, this->ch};
        }
    }

    this->readChar();
    return token;
}
