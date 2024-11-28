//
// Created by mizuk on 2024/11/28.
//

#include "parser.h"

#include "fmt/format.h"

void Parser::nextToken() {
    this->curToken = this->peekToken;
    this->peekToken = this->lexer.nextToken();
}

bool Parser::curTokenIs(const TokenType &t) const {
    return this->curToken.type == t;
}

bool Parser::peekTokenIs(const TokenType &t) const {
    return this->peekToken.type == t;
}

bool Parser::expectPeek(const TokenType &t) {
    if (this->peekTokenIs(t)) {
        this->nextToken();
        return true;
    }
    this->peekError(t);
    return false;
}

void Parser::peekError(TokenType t) {
    this->_errors.push_back(fmt::format("expected next token to be {:s}, got {:s} instead", t, this->peekToken.type));
}

void Parser::noPrefixParseFnError(TokenType t) {
    this->_errors.push_back(fmt::format("no prefix parse function for {:s} found", t));
}

std::unique_ptr<Ast::Statement> Parser::parseStatement() {
    if (this->curToken.type == LET) {
        return this->parseLetStatement();
    }
    if (this->curToken.type == RETURN) {
        return this->parseReturnStatement();
    }
    return this->parseExpressionStatement();
}

std::unique_ptr<Ast::LetStatement> Parser::parseLetStatement() {
    auto token = this->curToken;
    if (!this->expectPeek(IDENT)) {
        return nullptr;
    }

    auto name = std::make_unique<Ast::Identifier>(this->curToken, this->curToken.literal);

    if (!this->expectPeek(ASSIGN)) {
        return nullptr;
    }

    this->nextToken();

    auto value = parseExpression(Precedence::LOWEST);

    if (this->peekTokenIs(SEMICOLON)) {
        this->nextToken();
    }

    return std::make_unique<Ast::LetStatement>(token, std::move(name), std::move(value));
}

std::unique_ptr<Ast::ReturnStatement> Parser::parseReturnStatement() {
    auto token = this->curToken;
    this->nextToken();
    auto returnValue = this->parseExpression(Precedence::LOWEST);
    if (this->peekTokenIs(SEMICOLON)) {
        this->nextToken();
    }
    return std::make_unique<Ast::ReturnStatement>(token, std::move(returnValue));
}

std::unique_ptr<Ast::ExpressionStatement> Parser::parseExpressionStatement() {
    auto token = this->curToken;
    auto expression = this->parseExpression(Precedence::LOWEST);

    if (this->peekTokenIs(SEMICOLON)) {
        this->nextToken();
    }
    return std::make_unique<Ast::ExpressionStatement>(token, std::move(expression));
}

std::unique_ptr<Ast::Expression> Parser::parseExpression(Precedence precedence) {
    const auto prefixFn = this->prefixParseFns[this->curToken.type];
    if (prefixFn == nullptr) {
        this->noPrefixParseFnError(this->curToken.type);
        return nullptr;
    }
    auto leftExpression = prefixFn();

    while (!this->peekTokenIs(SEMICOLON) && precedence < this->peekPrecedence()) {
        auto infixFn = this->infixParseFns[this->peekToken.type];
        if (infixFn == nullptr) {
            return std::move(leftExpression);
        }

        this->nextToken();
        leftExpression = infixFn(std::move(leftExpression));
    }
    return leftExpression;
}

Precedence Parser::peekPrecedence() const {
    if (const auto iterator = precedences.find(this->peekToken.type); iterator != precedences.end()) {
        return precedences[this->peekToken.type];
    }
    return Precedence::LOWEST;
}

Precedence Parser::curPrecedence() const {
    if (const auto iterator = precedences.find(this->curToken.type); iterator != precedences.end()) {
        return precedences[this->curToken.type];
    }
    return Precedence::LOWEST;
}

std::unique_ptr<Ast::Expression> Parser::parseIdentifier() {
    return std::make_unique<Ast::Identifier>(this->curToken, this->curToken.literal);
}

std::unique_ptr<Ast::Expression> Parser::parseIntegerLiteral() {
    auto token = this->curToken;
    try {
        auto value = std::stoll(this->curToken.literal);
        return std::make_unique<Ast::IntegerLiteral>(token, value);
    } catch ([[maybe_unused]] const std::invalid_argument &e) {
    } catch ([[maybe_unused]] const std::out_of_range &e) {
        this->_errors.push_back(fmt::format("could not parse {} as integer", this->curToken.literal));
        return nullptr;
    }
    return nullptr;
}

std::unique_ptr<Ast::Expression> Parser::parseStringLiteral() const {
    return std::make_unique<Ast::StringLiteral>(std::move(Ast::StringLiteral{this->curToken, this->curToken.literal}));
}

std::unique_ptr<Ast::Expression> Parser::parsePrefixExpression() {
    auto token = this->curToken;
    auto operator_ = this->curToken.literal;

    this->nextToken();
    auto right = this->parseExpression(Precedence::PREFIX);

    return std::make_unique<Ast::PrefixExpression>(
        std::move(token),
        std::move(operator_),
        std::move(right)
    );
}

std::unique_ptr<Ast::Expression> Parser::parseInfixExpression(std::unique_ptr<Ast::Expression> left) {
    auto token = this->curToken;
    auto operator_ = this->curToken.literal;

    const auto precedence = this->curPrecedence();
    this->nextToken();
    auto right = this->parseExpression(precedence);

    return std::make_unique<Ast::InfixExpression>(
        std::move(token),
        std::move(left),
        std::move(operator_),
        std::move(right)
    );
}

std::unique_ptr<Ast::Expression> Parser::parseBoolean() {
    return std::make_unique<Ast::Boolean>(this->curToken, this->curTokenIs(TRUE));
}

std::unique_ptr<Ast::Expression> Parser::parseGroupedExpression() {
    this->nextToken();

    auto expression = this->parseExpression(Precedence::LOWEST);

    if (!this->expectPeek(RPAREN)) {
        return nullptr;
    }
    return expression;
}

std::unique_ptr<Ast::Expression> Parser::parseIfExpression() {
    auto token = this->curToken;

    if (!this->expectPeek(LPAREN)) {
        return nullptr;
    }

    this->nextToken();
    auto condition = this->parseExpression(Precedence::LOWEST);

    if (!this->expectPeek(RPAREN)) {
        return nullptr;
    }

    if (!this->expectPeek(LBRACE)) {
        return nullptr;
    }

    auto consequence = this->parseBlockStatement();

    if (this->peekTokenIs(ELSE)) {
        this->nextToken();

        if (!this->expectPeek(LBRACE)) {
            return nullptr;
        }

        auto alternative = this->parseBlockStatement();
        return std::make_unique<Ast::IfExpression>(token, std::move(condition), std::move(consequence),
                                                   std::move(alternative));
    }

    return std::make_unique<Ast::IfExpression>(token, std::move(condition), std::move(consequence), nullptr);
}

std::unique_ptr<Ast::BlockStatement> Parser::parseBlockStatement() {
    auto token = this->curToken;
    std::vector<std::unique_ptr<Ast::Statement> > statements{};

    this->nextToken();

    while (!this->curTokenIs(RBRACE) && !this->curTokenIs(EOF_)) {
        auto stmt = this->parseStatement();
        if (stmt != nullptr) {
            statements.push_back(std::move(stmt));
        }
        this->nextToken();
    }

    return std::make_unique<Ast::BlockStatement>(token, std::move(statements));
}

std::unique_ptr<Ast::Expression> Parser::parseFunctionLiteral() {
    auto token = this->curToken;

    if (!this->expectPeek(LPAREN)) {
        return nullptr;
    }

    auto parameters = this->parseFunctionParameters();

    if (!this->expectPeek(LBRACE)) {
        return nullptr;
    }

    auto body = this->parseBlockStatement();

    return std::make_unique<Ast::FunctionLiteral>(token, *parameters, std::move(body));
}

std::unique_ptr<std::vector<Ast::Identifier> > Parser::parseFunctionParameters() {
    if (this->peekTokenIs(RPAREN)) {
        this->nextToken();
        return std::make_unique<std::vector<Ast::Identifier> >();
    }

    this->nextToken();

    auto token = this->curToken;
    auto value = this->curToken.literal;

    std::vector<Ast::Identifier> identifiers{};
    identifiers.emplace_back(token, value);

    while (this->peekTokenIs(COMMA)) {
        this->nextToken();
        this->nextToken();
        auto identifier = std::make_unique<Ast::Identifier>(this->curToken, this->curToken.literal);
        identifiers.push_back(*identifier.get());
    }

    if (!this->expectPeek(RPAREN)) {
        return nullptr;
    }


    return std::make_unique<std::vector<Ast::Identifier> >(identifiers);
}

std::unique_ptr<Ast::Expression> Parser::parseCallExpression(std::unique_ptr<Ast::Expression> function) {
    return std::make_unique<Ast::CallExpression>(this->curToken, std::move(function),
                                                 std::move(*this->parseExpressionList(RPAREN)));
}

std::unique_ptr<std::vector<std::unique_ptr<Ast::Expression> > > Parser::parseExpressionList(const TokenType &end) {
    std::vector<std::unique_ptr<Ast::Expression> > expressions{};

    if (this->peekTokenIs(end)) {
        this->nextToken();
        return std::make_unique<std::vector<std::unique_ptr<Ast::Expression> > >();
    }

    this->nextToken();
    expressions.push_back(this->parseExpression(Precedence::LOWEST));

    while (this->peekTokenIs(COMMA)) {
        this->nextToken();
        this->nextToken();
        expressions.push_back(this->parseExpression(Precedence::LOWEST));
    }

    if (!this->expectPeek(end)) {
        return nullptr;
    }

    return std::make_unique<std::vector<std::unique_ptr<Ast::Expression> > >(std::move(expressions));
}

std::unique_ptr<Ast::Expression> Parser::parseArrayLiteral() {
    return std::make_unique<Ast::ArrayLiteral>(this->curToken, std::move(*this->parseExpressionList(RBRACKET).get()));
}

std::unique_ptr<Ast::Expression> Parser::parseIndexExpression(std::unique_ptr<Ast::Expression> left) {
    auto token = this->curToken;

    this->nextToken();
    auto index = this->parseExpression(Precedence::LOWEST);

    if (!this->expectPeek(RBRACKET)) {
        return nullptr;
    }
    return std::make_unique<Ast::IndexExpression>(token, std::move(left), std::move(index));
}

std::unique_ptr<Ast::Expression> Parser::parseHashLiteral() {
    auto token = this->curToken;
    std::vector<std::pair<std::unique_ptr<Ast::Expression>, std::unique_ptr<Ast::Expression>>> pairs;

    while (!this->peekTokenIs(RBRACE)) {
        this->nextToken();
        auto key = this->parseExpression(Precedence::LOWEST);
        if (!key) {
            return nullptr;
        }

        if (!this->expectPeek(COLON)) {
            return nullptr;
        }

        this->nextToken();
        auto value = this->parseExpression(Precedence::LOWEST);
        if (!value) {
            return nullptr;
        }

        pairs.push_back(std::make_pair(std::move(key), std::move(value)));

        if (!this->peekTokenIs(RBRACE) && !this->expectPeek(COMMA)) {
            return nullptr;
        }
    }

    if (!this->expectPeek(RBRACE)) {
        return nullptr;
    }

    return std::make_unique<Ast::HashLiteral>(token, std::move(pairs));
}

void Parser::registerPrefix(const TokenType &tokenType, const PrefixParseFn fn) {
    this->prefixParseFns.emplace(tokenType, std::move(fn));
}

void Parser::registerInfix(const TokenType &tokenType, const InfixParseFn fn) {
    this->infixParseFns.emplace(tokenType, std::move(fn));
}

std::unique_ptr<Ast::Program> Parser::parseProgram() {
    std::vector<std::unique_ptr<Ast::Statement> > statements;

    while (!this->curTokenIs(EOF_)) {
        if (auto stmt = this->parseStatement(); stmt != nullptr) {
            statements.push_back(std::move(stmt));
        }
        this->nextToken();
    }
    return std::make_unique<Ast::Program>(std::move(statements));
}

std::vector<std::string> Parser::errors() {
    return this->_errors;
}
