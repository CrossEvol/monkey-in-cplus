//
// Created by mizuk on 2024/11/27.
//

#ifndef AST_H
#define AST_H
#include <memory>
#include <string>
#include <utility>
#include <vector>
#include "../token/token.h"

namespace Ast {
    enum TypeID: uint8_t {
        Statement_,
        Expression_,
        Program_,
        Identifier_,
        LetStatement_,
        ReturnStatement_,
        ExpressionStatement_,
        BlockStatement_,
        Boolean_,
        IntegerLiteral_,
        PrefixExpression_,
        InfixExpression_,
        IfExpression_,
        FunctionLiteral_,
        CallExpression_,
        StringLiteral_,
        ArrayLiteral_,
        IndexExpression_,
        HashLiteral_,
    };

    class Node {
    public:
        virtual ~Node() = default;

        virtual std::string tokenLiteral() = 0;

        virtual std::string string() =0;

        virtual TypeID typeID() = 0;
    };

    class Statement : virtual public Node {
    public:
        ~Statement() override = default;

        void statementNode() {
        };

        std::string tokenLiteral() override = 0;

        std::string string() override;

        TypeID typeID() override;
    };

    class Expression : virtual public Node {
    public:
        ~Expression() override = default;

        void expressionNode() {
        };

        std::string tokenLiteral() override =0;

        std::string string() override = 0;

        TypeID typeID() override;
    };

    class Program final : public Node {
    public:
        std::vector<std::unique_ptr<Statement> > statements;

        ~Program() override = default;

        explicit Program(std::vector<std::unique_ptr<Statement> > &&statements)
            : statements(std::move(statements)) {
        }

        std::string tokenLiteral() override;

        std::string string() override;

        TypeID typeID() override;
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

        TypeID typeID() override;
    };

    class LetStatement final : public Statement {
    public:
        Token token;
        std::unique_ptr<Identifier> name;
        std::unique_ptr<Expression> value;

        LetStatement(Token token, std::unique_ptr<Identifier> name, std::unique_ptr<Expression> value)
            : token(std::move(token)),
              name(std::move(name)),
              value(std::move(value)) {
        }

        ~LetStatement() override = default;

        std::string tokenLiteral() override;

        std::string string() override;

        TypeID typeID() override;
    };

    class ReturnStatement final : public Statement {
    public:
        Token token;
        std::unique_ptr<Expression> returnValue;

        ReturnStatement(Token token, std::unique_ptr<Expression> return_value)
            : token(std::move(token)),
              returnValue(std::move(return_value)) {
        }

        ~ReturnStatement() override = default;

        std::string tokenLiteral() override;

        std::string string() override;

        TypeID typeID() override;
    };

    class ExpressionStatement final : public Statement {
    public:
        Token token;
        std::unique_ptr<Expression> expression;

        ExpressionStatement(Token token, std::unique_ptr<Expression> expression)
            : token(std::move(token)),
              expression(std::move(expression)) {
        }

        ~ExpressionStatement() override = default;

        std::string tokenLiteral() override;

        std::string string() override;

        TypeID typeID() override;
    };

    class BlockStatement final : public Statement {
    public:
        Token token;
        std::vector<std::unique_ptr<Statement> > statements;

        BlockStatement(Token token, std::vector<std::unique_ptr<Statement> > &&statements)
            : token(std::move(token)), statements(std::move(statements)) {
        }

        BlockStatement(const BlockStatement &) = delete;

        BlockStatement &operator=(const BlockStatement &) = delete;

        BlockStatement(BlockStatement &&) = default;

        BlockStatement &operator=(BlockStatement &&) = default;

        ~BlockStatement() override = default;

        std::string tokenLiteral() override;

        std::string string() override;

        TypeID typeID() override;
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

        TypeID typeID() override;
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

        TypeID typeID() override;
    };

    class PrefixExpression final : public Expression {
    public:
        Token token;
        std::string operator_;
        std::unique_ptr<Expression> right;

        PrefixExpression(Token token, std::string op, std::unique_ptr<Expression> right)
            : token(std::move(token)), operator_(std::move(op)), right(std::move(right)) {
        }

        ~PrefixExpression() override = default;

        std::string tokenLiteral() override;

        std::string string() override;

        TypeID typeID() override;
    };

    class InfixExpression final : public Expression {
    public:
        Token token;
        std::unique_ptr<Expression> left;
        std::string operator_;
        std::unique_ptr<Expression> right;

        InfixExpression(Token token, std::unique_ptr<Expression> left, std::string op,
                        std::unique_ptr<Expression> right)
            : token(std::move(token)), left(std::move(left)), operator_(std::move(op)), right(std::move(right)) {
        }

        ~InfixExpression() override = default;

        std::string tokenLiteral() override;

        std::string string() override;

        TypeID typeID() override;
    };

    class IfExpression final : public Expression {
    public:
        Token token;
        std::unique_ptr<Expression> condition;
        std::unique_ptr<BlockStatement> consequence;
        std::unique_ptr<BlockStatement> alternative;

        IfExpression(Token token,
                     std::unique_ptr<Expression> condition,
                     std::unique_ptr<BlockStatement> consequence,
                     std::unique_ptr<BlockStatement> alternative)
            : token(std::move(token)),
              condition(std::move(condition)),
              consequence(std::move(consequence)),
              alternative(std::move(alternative)) {
        }

        ~IfExpression() override = default;

        std::string tokenLiteral() override;

        std::string string() override;

        TypeID typeID() override;
    };

    class FunctionLiteral final : public Expression {
    public:
        Token token;
        std::vector<Identifier> parameters;
        std::unique_ptr<BlockStatement> body;

        FunctionLiteral(Token token, std::vector<Identifier> parameters, std::unique_ptr<BlockStatement> body)
            : token(std::move(token)),
              parameters(std::move(parameters)),
              body(std::move(body)) {
        }

        ~FunctionLiteral() override = default;

        std::string tokenLiteral() override;

        std::string string() override;

        TypeID typeID() override;
    };

    class CallExpression final : public Expression {
    public:
        Token token;
        std::unique_ptr<Expression> function;
        std::vector<std::unique_ptr<Expression> > arguments;

        CallExpression(Token token,
                       std::unique_ptr<Expression> function,
                       std::vector<std::unique_ptr<Expression> > &&arguments)
            : token(std::move(token)),
              function(std::move(function)),
              arguments(std::move(arguments)) {
        }

        ~CallExpression() override = default;

        std::string tokenLiteral() override;

        std::string string() override;

        TypeID typeID() override;
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

        TypeID typeID() override;
    };

    class ArrayLiteral final : public Expression {
    public:
        Token token;
        std::vector<std::unique_ptr<Expression> > elements;

        ArrayLiteral(Token token, std::vector<std::unique_ptr<Expression> > elements)
            : token(std::move(token)), elements(std::move(elements)) {
        }

        ~ArrayLiteral() override = default;

        std::string tokenLiteral() override;

        std::string string() override;

        TypeID typeID() override;
    };

    class IndexExpression final : public Expression {
    public:
        Token token;
        std::unique_ptr<Expression> left;
        std::unique_ptr<Expression> index;

        IndexExpression(Token token, std::unique_ptr<Expression> left, std::unique_ptr<Expression> index)
            : token(std::move(token)), left(std::move(left)), index(std::move(index)) {
        }

        ~IndexExpression() override = default;

        std::string tokenLiteral() override;

        std::string string() override;

        TypeID typeID() override;
    };

    class HashLiteral final : public Expression {
    public:
        Token token;
        std::map<std::string, std::pair<std::unique_ptr<Expression>, std::unique_ptr<Expression> > > pairs;

        HashLiteral(Token token,
                    std::map<std::string, std::pair<std::unique_ptr<Expression>, std::unique_ptr<Expression> > > &&
                    pairs)
            : token(std::move(token)), pairs(std::move(pairs)) {
        }

        ~HashLiteral() override = default;

        std::string tokenLiteral() override;

        std::string string() override;

        Expression *get(Expression &left);

        TypeID typeID() override;
    };
}

#endif //AST_H
