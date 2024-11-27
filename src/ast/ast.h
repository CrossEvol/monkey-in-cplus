//
// Created by mizuk on 2024/11/27.
//

#ifndef AST_H
#define AST_H
#include <string>
#include <utility>
#include <vector>
#include "../token/token.h"

namespace Ast {
    class Node {
    public:
        virtual ~Node() = default;

        virtual std::string tokenLiteral() { return "???"; }

        virtual std::string string() { return "???"; }
    };

    class Statement : public Node {
    public:
        ~Statement() override = default;

        void statementNode() {
        };

        virtual std::string tokenLiteral() { return "???"; }

        virtual std::string string() { return "???"; }
    };

    class Expression : public Node {
    public:
        ~Expression() override = default;

        void expressionNode() {
        };

        virtual std::string tokenLiteral() { return "???"; }

        virtual std::string string() { return "???"; }
    };

    class Program final : public Node {
    private:
        std::vector<Statement> statements;

    public:
        ~Program() override = default;

        explicit Program(const std::vector<Statement> &statements)
            : statements(statements) {
        }

        std::string tokenLiteral() override;

        std::string string() override;
    };

    class Identifier final : public Expression {
    public:
        Token token;
        std::string value;

        Identifier(Token token, std::string value)
            : token(std::move(token)),
              value(std::move(value)) {
        }

        ~Identifier() override = default;

        std::string tokenLiteral() override;

        std::string string() override;
    };

    class LetStatement final : public Statement {
    public:
        Token token;
        Identifier *name;
        Expression *value;

        LetStatement(Token token, Identifier *name, Expression *value)
            : token(std::move(token)),
              name(std::move(name)),
              value(value) {
        }

        ~LetStatement() override = default;

        std::string tokenLiteral() override;

        std::string string() override;
    };

    class ReturnStatement final : public Statement {
    public:
        Token token;
        Expression *returnValue;

        ReturnStatement(Token token, Expression *return_value)
            : token(std::move(token)),
              returnValue(return_value) {
        }

        ~ReturnStatement() override = default;

        std::string tokenLiteral() override;

        std::string string() override;
    };

    class ExpressionStatement final : public Statement {
    public:
        Token token;
        Expression *expression;

        ExpressionStatement(Token token, Expression *expression)
            : token(std::move(token)),
              expression(expression) {
        }

        ~ExpressionStatement() override = default;

        std::string tokenLiteral() override;

        std::string string() override;
    };

    class BlockStatement final : public Statement {
    public:
        Token token;
        std::vector<Statement> statements;

        BlockStatement(Token token, const std::vector<Statement> &statements)
            : token(std::move(token)),
              statements(statements) {
        }

        ~BlockStatement() override = default;

        std::string tokenLiteral() override;

        std::string string() override;
    };

    class Boolean final : public Expression {
    public:
        Token token;
        bool value;

        Boolean(Token token, bool value)
            : token(std::move(token)),
              value(value) {
        }

        ~Boolean() override = default;

        std::string tokenLiteral() override;

        std::string string() override;
    };

    class IntegerLiteral final : public Expression {
    public:
        Token token;
        std::int64_t value;

        IntegerLiteral(Token token, std::int64_t value)
            : token(std::move(token)),
              value(value) {
        }

        ~IntegerLiteral() override = default;

        std::string tokenLiteral() override;

        std::string string() override;
    };

    class PrefixExpression final : public Expression {
    public:
        Token token;
        std::string operator_;
        Expression *right;

        PrefixExpression(Token token, std::string op, Expression *right)
            : token(std::move(token)), operator_(std::move(op)), right(right) {
        }

        ~PrefixExpression() override = default;

        std::string tokenLiteral() override;

        std::string string() override;
    };

    class InfixExpression final : public Expression {
    public:
        Token token;
        Expression *left;
        std::string operator_;
        Expression *right;

        InfixExpression(Token token, Expression *left, std::string op, Expression *right)
            : token(std::move(token)), left(left), operator_(std::move(op)), right(right) {
        }

        ~InfixExpression() override = default;

        std::string tokenLiteral() override;

        std::string string() override;
    };

    class IfExpression final : public Expression {
    public:
        Token token;
        Expression *condition;
        BlockStatement *consequence;
        BlockStatement *alternative;

        IfExpression(Token token, Expression *condition, BlockStatement *consequence, BlockStatement *alternative)
            : token(std::move(token)), condition(condition), consequence(consequence), alternative(alternative) {
        }

        ~IfExpression() override = default;

        std::string tokenLiteral() override;

        std::string string() override;
    };

    class FunctionLiteral final : public Expression {
    public:
        Token token;
        std::vector<Identifier> parameters;
        BlockStatement *body;

        FunctionLiteral(Token token, const std::vector<Identifier> &parameters, BlockStatement *body)
            : token(std::move(token)), parameters(parameters), body(body) {
        }

        ~FunctionLiteral() override = default;

        std::string tokenLiteral() override;

        std::string string() override;
    };

    class CallExpression final : public Expression {
    public:
        Token token;
        Expression *function;
        std::vector<Expression *> arguments;

        CallExpression(Token token, Expression *function, const std::vector<Expression *> &arguments)
            : token(std::move(token)), function(function), arguments(arguments) {
        }

        ~CallExpression() override = default;

        std::string tokenLiteral() override;

        std::string string() override;
    };

    class StringLiteral final : public Expression {
    public:
        Token token;
        std::string value;

        StringLiteral(Token token, std::string value)
            : token(std::move(token)), value(std::move(value)) {
        }

        ~StringLiteral() override = default;

        std::string tokenLiteral() override;

        std::string string() override;
    };

    class ArrayLiteral final : public Expression {
    public:
        Token token;
        std::vector<Expression *> elements;

        ArrayLiteral(Token token, const std::vector<Expression *> &elements)
            : token(std::move(token)), elements(elements) {
        }

        ~ArrayLiteral() override = default;

        std::string tokenLiteral() override;

        std::string string() override;
    };

    class IndexExpression final : public Expression {
    public:
        Token token;
        Expression *left;
        Expression *index;

        IndexExpression(Token token, Expression *left, Expression *index)
            : token(std::move(token)), left(left), index(index) {
        }

        ~IndexExpression() override = default;

        std::string tokenLiteral() override;

        std::string string() override;
    };

    class HashLiteral final : public Expression {
    public:
        Token token;
        std::map<Expression *, Expression *> pairs;

        HashLiteral(Token token, const std::map<Expression *, Expression *> &pairs)
            : token(std::move(token)), pairs(pairs) {
        }

        ~HashLiteral() override = default;

        std::string tokenLiteral() override;

        std::string string() override;
    };
}

#endif //AST_H
