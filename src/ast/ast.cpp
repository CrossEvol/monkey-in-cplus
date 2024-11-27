//
// Created by mizuk on 2024/11/27.
//

#include "ast.h"
#include <sstream>

std::string Program::tokenLiteral() {
    if (!this->statements.empty()) {
        auto statement = this->statements[0];
        return statement.tokenLiteral();
    }
    return "";
}

std::string Program::string() {
    std::ostringstream oss;
    for (auto s: this->statements) {
        oss << s.string();
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
    std::ostringstream oss;
    oss << this->tokenLiteral() << " ";
    oss << this->name->string();
    oss << " = ";
    if (this->value != nullptr) {
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
    if (this->returnValue != nullptr) {
        oss << this->returnValue->string();
    }
    oss << ";";
    return oss.str();
}

std::string ExpressionStatement::tokenLiteral() {
    return this->token.literal;
}

std::string ExpressionStatement::string() {
    if (this->expression != nullptr) {
        return this->expression->string();
    }
    return "";
}

std::string BlockStatement::tokenLiteral() {
    return this->token.literal;
}

std::string BlockStatement::string() {
    std::ostringstream oss;
    for (auto s: this->statements) {
        oss << s.string();
    }
    return oss.str();
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
    if (this->right != nullptr) {
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
    if (this->left != nullptr) {
        oss << this->left->string();
    }
    oss << " " << this->operator_ << " ";
    if (this->right != nullptr) {
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
    if (this->condition != nullptr) {
        oss << this->condition->string();
    }
    oss << " ";
    if (this->consequence != nullptr) {
        oss << this->consequence->string();
    }
    if (this->alternative != nullptr) {
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
    if (this->body != nullptr) {
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

    for (const auto argument : this->arguments) {
        if(argument != nullptr) {
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
        if (el != nullptr) {
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
    if (this->left != nullptr) {
        oss << this->left->string();
    }
    oss << "[";
    if (this->index != nullptr) {
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

    for (const auto &[fst, snd]: this->pairs) {
        if (fst != nullptr && snd != nullptr) {
            pairs.push_back(fst->string() + ":" + snd->string());
        }
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
