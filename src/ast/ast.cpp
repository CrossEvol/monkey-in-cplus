//
// Created by mizuk on 2024/11/27.
//

#include "ast.h"
#include <sstream>

using namespace Ast;

std::string Statement::string() {
    return {};
}

std::string Program::tokenLiteral() {
    if (!this->statements.empty()) {
        auto statement = std::move(this->statements[0]);
        return statement->tokenLiteral();
    }
    return "";
}

std::string Program::string() {
    std::ostringstream oss;
    for (const auto &s: this->statements) {
        oss << s->string();
    }
    return oss.str();
}

std::string Identifier::tokenLiteral() {
    return this->token.literal;
}

std::string Identifier::string() {
    return this->value;
}

std::string LetStatement::tokenLiteral() {
    return this->token.literal;
}

std::string LetStatement::string() {
    std::stringstream oss;
    oss << this->tokenLiteral() << " ";
    oss << this->name->string();
    oss << " = ";
    if (this->value) {
        oss << this->value->string();
    }
    oss << ";";
    return oss.str();
}

std::string ReturnStatement::tokenLiteral() {
    return this->token.literal;
}

std::string ReturnStatement::string() {
    std::ostringstream oss;
    oss << this->tokenLiteral() << " ";
    if (this->returnValue) {
        oss << this->returnValue->string();
    }
    oss << ";";
    return oss.str();
}

std::string ExpressionStatement::tokenLiteral() {
    return this->token.literal;
}

std::string ExpressionStatement::string() {
    if (this->expression) {
        return this->expression->string();
    }
    return "";
}

std::string BlockStatement::tokenLiteral() {
    return this->token.literal;
}

std::string BlockStatement::string() {
    std::string out;
    for (const auto &s: this->statements) {
        out += s->string();
    }
    return out;
}

std::string Boolean::tokenLiteral() {
    return this->token.literal;
}

std::string Boolean::string() {
    return this->token.literal;
}

std::string IntegerLiteral::tokenLiteral() {
    return this->token.literal;
}

std::string IntegerLiteral::string() {
    return this->token.literal;
}

std::string PrefixExpression::tokenLiteral() {
    return this->token.literal;
}

std::string PrefixExpression::string() {
    std::ostringstream oss;
    oss << "(";
    oss << this->operator_;
    if (this->right) {
        oss << this->right->string();
    }
    oss << ")";
    return oss.str();
}

std::string InfixExpression::tokenLiteral() {
    return this->token.literal;
}

std::string InfixExpression::string() {
    std::ostringstream oss;
    oss << "(";
    if (this->left) {
        oss << this->left->string();
    }
    oss << " " << this->operator_ << " ";
    if (this->right) {
        oss << this->right->string();
    }
    oss << ")";
    return oss.str();
}

std::string IfExpression::tokenLiteral() {
    return this->token.literal;
}

std::string IfExpression::string() {
    std::ostringstream oss;
    oss << "if";
    if (this->condition) {
        oss << this->condition->string();
    }
    oss << " ";
    if (this->consequence) {
        oss << this->consequence->string();
    }
    if (this->alternative) {
        oss << "else ";
        oss << this->alternative->string();
    }
    return oss.str();
}

std::string FunctionLiteral::tokenLiteral() {
    return this->token.literal;
}

std::string FunctionLiteral::string() {
    std::ostringstream oss;
    std::vector<std::string> params;

    params.reserve(this->parameters.size());
    for (auto &p: this->parameters) {
        params.push_back(p.string());
    }

    oss << this->tokenLiteral();
    oss << "(";

    for (auto i = 0; i < params.size(); ++i) {
        oss << params[i];
        if (i < params.size() - 1) {
            oss << ", ";
        }
    }

    oss << ") ";
    if (this->body) {
        oss << this->body->string();
    }
    return oss.str();
}

std::string CallExpression::tokenLiteral() {
    return this->token.literal;
}

std::string CallExpression::string() {
    std::ostringstream oss;
    std::vector<std::string> args;

    if (this->function != nullptr) {
        oss << this->function->string();
    }

    oss << "(";

    for (const auto &argument: this->arguments) {
        if (argument != nullptr) {
            args.push_back(argument->string());
        }
    }

    for (auto i = 0; i < args.size(); ++i) {
        oss << args[i];
        if (i < args.size() - 1) {
            oss << ", ";
        }
    }

    oss << ")";
    return oss.str();
}

std::string StringLiteral::tokenLiteral() {
    return this->token.literal;
}

std::string StringLiteral::string() {
    return this->token.literal;
}

std::string ArrayLiteral::tokenLiteral() {
    return this->token.literal;
}

std::string ArrayLiteral::string() {
    std::ostringstream oss;
    std::vector<std::string> elements;

    for (const auto &el: this->elements) {
        if (el) {
            elements.push_back(el->string());
        }
    }

    oss << "[";
    for (size_t i = 0; i < elements.size(); ++i) {
        oss << elements[i];
        if (i < elements.size() - 1) {
            oss << ", ";
        }
    }
    oss << "]";

    return oss.str();
}

std::string IndexExpression::tokenLiteral() {
    return this->token.literal;
}

std::string IndexExpression::string() {
    std::ostringstream oss;
    oss << "(";
    if (this->left) {
        oss << this->left->string();
    }
    oss << "[";
    if (this->index) {
        oss << this->index->string();
    }
    oss << "])";
    return oss.str();
}

std::string HashLiteral::tokenLiteral() {
    return this->token.literal;
}

std::string HashLiteral::string() {
    std::ostringstream oss;
    std::vector<std::string> pairs;

    for (const auto &[_, pair]: this->pairs) {
        pairs.push_back(pair.first->string() + ":" + pair.second->string());
    }

    oss << "{";
    for (size_t i = 0; i < pairs.size(); ++i) {
        oss << pairs[i];
        if (i < pairs.size() - 1) {
            oss << ", ";
        }
    }
    oss << "}";

    return oss.str();
}

Expression* HashLiteral::get( Expression &left) {
    const auto hash_key = std::to_string(std::hash<std::string>{}(left.string()));
    if(this->pairs.find(hash_key) == this->pairs.end()) {
        return nullptr;
    }
    // TODO: should use move here ?
    return this->pairs.at(hash_key).second.get();
}


TypeID Statement::typeID() { 
    return TypeID::Statement_; 
}

TypeID Expression::typeID() { 
    return TypeID::Expression_; 
}

TypeID Program::typeID() { 
    return TypeID::Program_; 
}

TypeID Identifier::typeID() { 
    return TypeID::Identifier_; 
}

TypeID LetStatement::typeID() { 
    return TypeID::LetStatement_; 
}

TypeID ReturnStatement::typeID() { 
    return TypeID::ReturnStatement_; 
}

TypeID ExpressionStatement::typeID() { 
    return TypeID::ExpressionStatement_; 
}

TypeID BlockStatement::typeID() { 
    return TypeID::BlockStatement_; 
}

TypeID Boolean::typeID() { 
    return TypeID::Boolean_; 
}

TypeID IntegerLiteral::typeID() { 
    return TypeID::IntegerLiteral_; 
}

TypeID PrefixExpression::typeID() { 
    return TypeID::PrefixExpression_; 
}

TypeID InfixExpression::typeID() { 
    return TypeID::InfixExpression_; 
}

TypeID IfExpression::typeID() { 
    return TypeID::IfExpression_; 
}

TypeID FunctionLiteral::typeID() { 
    return TypeID::FunctionLiteral_; 
}

TypeID CallExpression::typeID() { 
    return TypeID::CallExpression_; 
}

TypeID StringLiteral::typeID() { 
    return TypeID::StringLiteral_; 
}

TypeID ArrayLiteral::typeID() { 
    return TypeID::ArrayLiteral_; 
}

TypeID IndexExpression::typeID() { 
    return TypeID::IndexExpression_; 
}

TypeID HashLiteral::typeID() { 
    return TypeID::HashLiteral_; 
}
