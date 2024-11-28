//
// Created by mizuk on 2024/11/28.
//

#ifndef PARSER_H
#define PARSER_H

#include <functional>
#include <utility>
#include <memory>

#include "../ast/ast.h"
#include "../lexer/lexer.h"
#include "../token/token.h"

enum class Precedence:unsigned int {
    _,
    LOWEST,
    EQUALS,
    LESS_GREATER,
    SUM,
    PRODUCT,
    PREFIX,
    CALL,
    INDEX,
};

inline std::map<TokenType, Precedence> precedences = {
    {EQ, Precedence::EQUALS},
    {NOT_EQ, Precedence::EQUALS},
    {LT, Precedence::LESS_GREATER},
    {GT, Precedence::LESS_GREATER},
    {PLUS, Precedence::SUM},
    {MINUS, Precedence::SUM},
    {SLASH, Precedence::PRODUCT},
    {ASTERISK, Precedence::PRODUCT},
    {LPAREN, Precedence::CALL},
    {LBRACKET, Precedence::INDEX},
};

using PrefixParseFn = std::function<std::unique_ptr<Ast::Expression>()>;
using InfixParseFn = std::function<std::unique_ptr<Ast::Expression>(std::unique_ptr<Ast::Expression>)>;

class Parser {
    Lexer lexer;
    std::vector<std::string> _errors{};

    Token curToken{};
    Token peekToken{};

    std::map<TokenType, PrefixParseFn> prefixParseFns{};
    std::map<TokenType, InfixParseFn> infixParseFns{};

    void nextToken();

    bool curTokenIs(const TokenType &t) const;

    bool peekTokenIs(const TokenType &t) const;

    bool expectPeek(const TokenType &t);

    void peekError(TokenType t);

    void noPrefixParseFnError(TokenType t);

    std::unique_ptr<Ast::Statement> parseStatement();

    std::unique_ptr<Ast::LetStatement> parseLetStatement();

    std::unique_ptr<Ast::ReturnStatement> parseReturnStatement();

    std::unique_ptr<Ast::ExpressionStatement> parseExpressionStatement();

    std::unique_ptr<Ast::Expression> parseExpression(Precedence precedence);

    Precedence peekPrecedence() const;

    Precedence curPrecedence() const;

    std::unique_ptr<Ast::Expression> parseIdentifier();

    std::unique_ptr<Ast::Expression> parseIntegerLiteral();

    std::unique_ptr<Ast::Expression> parseStringLiteral() const;

    std::unique_ptr<Ast::Expression> parsePrefixExpression();

    std::unique_ptr<Ast::Expression> parseInfixExpression(std::unique_ptr<Ast::Expression> left);

    std::unique_ptr<Ast::Expression> parseBoolean();

    std::unique_ptr<Ast::Expression> parseGroupedExpression();

    std::unique_ptr<Ast::Expression> parseIfExpression();

    std::unique_ptr<Ast::BlockStatement> parseBlockStatement();

    std::unique_ptr<Ast::Expression> parseFunctionLiteral();

    std::unique_ptr<std::vector<Ast::Identifier> > parseFunctionParameters();

    std::unique_ptr<Ast::Expression> parseCallExpression(std::unique_ptr<Ast::Expression> function);

    std::unique_ptr<std::vector<std::unique_ptr<Ast::Expression> > > parseExpressionList(const TokenType &end);

    std::unique_ptr<Ast::Expression> parseArrayLiteral();

    std::unique_ptr<Ast::Expression> parseIndexExpression(std::unique_ptr<Ast::Expression> left);

    std::unique_ptr<Ast::Expression> parseHashLiteral();

    void registerPrefix(const TokenType &tokenType, PrefixParseFn fn);

    void registerInfix(const TokenType &tokenType, InfixParseFn fn);

public:
    explicit Parser(Lexer lexer) : lexer(std::move(lexer)) {
        this->registerPrefix(IDENT, [this] { return parseIdentifier(); });
        this->registerPrefix(INT, [this
                             ] {
                                 return parseIntegerLiteral();
                             });
        this->registerPrefix(STRING, [this] { return parseStringLiteral(); });
        this->registerPrefix(BANG, [this] { return parsePrefixExpression(); });
        this->registerPrefix(MINUS, [this] { return parsePrefixExpression(); });
        this->registerPrefix(TRUE, [this] { return parseBoolean(); });
        this->registerPrefix(FALSE, [this] { return parseBoolean(); });
        this->registerPrefix(LPAREN, [this] { return parseGroupedExpression(); });
        this->registerPrefix(IF, [this] { return parseIfExpression(); });
        this->registerPrefix(FUNCTION, [this] { return parseFunctionLiteral(); });
        this->registerPrefix(LBRACKET, [this] { return parseArrayLiteral(); });
        this->registerPrefix(LBRACE, [this] { return parseHashLiteral(); });

        this->registerInfix(PLUS, std::bind(&Parser::parseInfixExpression, this, std::placeholders::_1));
        this->registerInfix(MINUS, std::bind(&Parser::parseInfixExpression, this, std::placeholders::_1));
        this->registerInfix(SLASH, std::bind(&Parser::parseInfixExpression, this, std::placeholders::_1));
        this->registerInfix(ASTERISK, std::bind(&Parser::parseInfixExpression, this, std::placeholders::_1));
        this->registerInfix(EQ, std::bind(&Parser::parseInfixExpression, this, std::placeholders::_1));
        this->registerInfix(NOT_EQ, std::bind(&Parser::parseInfixExpression, this, std::placeholders::_1));
        this->registerInfix(LT, std::bind(&Parser::parseInfixExpression, this, std::placeholders::_1));
        this->registerInfix(GT, std::bind(&Parser::parseInfixExpression, this, std::placeholders::_1));

        this->registerInfix(LPAREN, std::bind(&Parser::parseCallExpression, this, std::placeholders::_1));
        this->registerInfix(LBRACKET, std::bind(&Parser::parseIndexExpression, this, std::placeholders::_1));

        this->nextToken();
        this->nextToken();
    }

    std::unique_ptr<Ast::Program> parseProgram();

    std::vector<std::string> errors();
};

#endif //PARSER_H
