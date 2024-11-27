//
// Created by mizuk on 2024/11/27.
//

#ifndef TOKEN_H
#define TOKEN_H
#include <map>
#include <string>

using TokenType = std::string;

namespace TokenType_t {
    inline TokenType ILLEGAL = "ILLEGAL";
    inline TokenType EOF_ = "EOF";

    // Identifiers + literals
    inline TokenType IDENT = "IDENT"; // add, foobar, x, y, ...
    inline TokenType INT = "INT"; // 1343456
    inline TokenType STRING = "STRING"; // "foobar"

    // Operators
    inline TokenType ASSIGN = "=";
    inline TokenType PLUS = "+";
    inline TokenType MINUS = "-";
    inline TokenType BANG = "!";
    inline TokenType ASTERISK = "*";
    inline TokenType SLASH = "/";

    inline TokenType LT = "<";
    inline TokenType GT = ">";

    inline TokenType EQ = "==";
    inline TokenType NOT_EQ = "!=";

    // Delimiters
    inline TokenType COMMA = ",";
    inline TokenType SEMICOLON = ";";
    inline TokenType COLON = ":";

    inline TokenType LPAREN = "(";
    inline TokenType RPAREN = ")";
    inline TokenType LBRACE = "{";
    inline TokenType RBRACE = "}";
    inline TokenType LBRACKET = "[";
    inline TokenType RBRACKET = "]";

    // Keywords
    inline TokenType FUNCTION = "FUNCTION";
    inline TokenType LET = "LET";
    inline TokenType TRUE = "TRUE";
    inline TokenType FALSE = "FALSE";
    inline TokenType IF = "IF";
    inline TokenType ELSE = "ELSE";
    inline TokenType RETURN = "RETURN";
}

struct Token {
    TokenType type;
    std::string literal;
};

using namespace TokenType_t;

inline std::map<std::string, TokenType> keywords{
    {"fn", FUNCTION},
    {"let", LET},
    {"true", TRUE},
    {"false", FALSE},
    {"if", IF},
    {"else", ELSE},
    {"return", RETURN}
};

TokenType lookupIdent(std::string_view ident);

#endif //TOKEN_H
