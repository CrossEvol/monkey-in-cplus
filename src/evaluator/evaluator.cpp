//
// Created by mizuk on 2024/12/4.
//

#include "evaluator.h"

#include <stdexcept>

#include "../common/common.h"
#include "../object/builtins.h"
#include "fmt/format.h"

Boolean *Evaluator::True = new Boolean(true);
Boolean *Evaluator::False = new Boolean(false);
OBJ::Null *Evaluator::Null = new OBJ::Null();

std::map<std::string, Builtin *> Evaluator::builtins{
    {"len", getBuiltinByName("len")},
    {"puts", getBuiltinByName("puts")},
    {"first", getBuiltinByName("first")},
    {"last", getBuiltinByName("last")},
    {"rest", getBuiltinByName("rest")},
    {"push", getBuiltinByName("push")},
};

Object *Evaluator::Eval(Ast::Node &_node, Environment &env) {
    if (instance_of<Ast::Node, Ast::Program>(_node)) {
        const auto node = dynamic_cast<Ast::Program *>(&_node);
        return this->evalProgram(*node, env);
    }
    if (instance_of<Ast::Node, Ast::BlockStatement>(_node)) {
        const auto node = dynamic_cast<Ast::BlockStatement *>(&_node);
        return this->evalBlockStatement(*node, env);
    }
    if (instance_of<Ast::Node, Ast::ExpressionStatement>(_node)) {
        const auto node = dynamic_cast<Ast::ExpressionStatement *>(&_node);
        return this->Eval(*node->expression, env);
    }
    if (instance_of<Ast::Node, Ast::ReturnStatement>(_node)) {
        const auto node = dynamic_cast<Ast::ReturnStatement *>(&_node);
        auto val = this->Eval(*node->returnValue, env);
        if (isError(val)) {
            return val;
        }
        return new ReturnValue(*val);
    }
    if (instance_of<Ast::Node, Ast::LetStatement>(_node)) {
        const auto node = dynamic_cast<Ast::LetStatement *>(&_node);
        auto val = this->Eval(*node->value, env);
        if (isError(val)) {
            return val;
        }
        env.set(node->name->value, *val);
    }
    if (instance_of<Ast::Node, Ast::IntegerLiteral>(_node)) {
        const auto node = dynamic_cast<Ast::IntegerLiteral *>(&_node);
        return new Integer(node->value);
    }
    if (instance_of<Ast::Node, Ast::StringLiteral>(_node)) {
        const auto node = dynamic_cast<Ast::StringLiteral *>(&_node);
        return new String(node->value);
    }
    if (instance_of<Ast::Node, Ast::Boolean>(_node)) {
        const auto node = dynamic_cast<Ast::Boolean *>(&_node);
        return this->nativeBoolToBooleanObject(node->value);
    }
    if (instance_of<Ast::Node, Ast::PrefixExpression>(_node)) {
        const auto node = dynamic_cast<Ast::PrefixExpression *>(&_node);
        auto right = this->Eval(*node->right, env);
        if (isError(right)) {
            return right;
        }
        return this->evalPrefixExpression(node->operator_, *right);
    }
    if (instance_of<Ast::Node, Ast::InfixExpression>(_node)) {
        const auto node = dynamic_cast<Ast::InfixExpression *>(&_node);
        auto left = this->Eval(*node->left, env);
        if (isError(left)) {
            return left;
        }

        auto right = this->Eval(*node->right, env);
        if (isError(right)) {
            return right;
        }
        return this->evalInfixExpression(node->operator_, *left, *right);
    }
    if (instance_of<Ast::Node, Ast::IfExpression>(_node)) {
        const auto node = dynamic_cast<Ast::IfExpression *>(&_node);
        return this->evalIfExpression(*node, env);
    }
    if (instance_of<Ast::Node, Ast::Identifier>(_node)) {
        const auto node = dynamic_cast<Ast::Identifier *>(&_node);
        return this->evalIdentifier(*node, env);
    }
    if (instance_of<Ast::Node, Ast::FunctionLiteral>(_node)) {
        const auto node = dynamic_cast<Ast::FunctionLiteral *>(&_node);
        std::vector<std::shared_ptr<Ast::Identifier> > params{};
        for (const auto &param: node->parameters) {
            params.push_back(std::make_shared<Ast::Identifier>(std::move(param)));
        }
        return new Function(params, std::make_shared<Ast::BlockStatement>(std::move(*node->body)),
                            std::make_shared<Environment>(env));
    }
    if (instance_of<Ast::Node, Ast::CallExpression>(_node)) {
        const auto node = dynamic_cast<Ast::CallExpression *>(&_node);
        auto function = this->Eval(*node->function, env);

        if (isError(function)) {
            return function;
        }

        std::vector<Ast::Expression *> arguments{};
        for (const auto &arg: node->arguments) {
            arguments.push_back(std::move(arg.get()));
        }
        auto args = this->evalExpressions(arguments, env);
        if (args.size() == 1 && isError(args[0])) {
            return args[0];
        }

        return this->applyFunction(*function, args);
    }
    if (instance_of<Ast::Node, Ast::ArrayLiteral>(_node)) {
        const auto node = dynamic_cast<Ast::ArrayLiteral *>(&_node);

        std::vector<Ast::Expression *> _elements{};
        for (const auto &element: node->elements) {
            _elements.push_back(std::move(element.get()));
        }
        auto elements = this->evalExpressions(_elements, env);
        return new Array(elements);
    }
    if (instance_of<Ast::Node, Ast::IndexExpression>(_node)) {
        const auto node = dynamic_cast<Ast::IndexExpression *>(&_node);

        auto left = this->Eval(*node->left, env);
        if (isError(left)) {
            return left;
        }
        auto index = this->Eval(*node->index, env);
        if (isError(index)) {
            return index;
        }
        return this->evalIndexExpression(*left, *index);
    }
    if (instance_of<Ast::Node, Ast::HashLiteral>(_node)) {
        const auto node = dynamic_cast<Ast::HashLiteral *>(&_node);
        return this->evalHashLiteral(*node, env);
    }

    return nullptr;
}

Object *Evaluator::evalProgram(Ast::Program &program, Environment &env) {
    Object *result = nullptr;

    for (auto &statement: program.statements) {
        result = this->Eval(*statement.get(), env);

        if (instance_of<Object, ReturnValue>(*result)) {
            return dynamic_cast<ReturnValue *>(result)->value;
        }
        if (instance_of<Object, Error>(*result)) {
            return dynamic_cast<Error *>(result);
        }
    }

    return result;
}

Object *Evaluator::evalBlockStatement(Ast::BlockStatement &block, Environment &env) {
    Object *result = nullptr;

    for (auto &statement: block.statements) {
        result = this->Eval(*statement.get(), env);
        if (result != nullptr) {
            auto rt = result->type();
            if (rt == RETURN_VALUE_OBJ || rt == ERROR_OBJ) {
                return result;
            }
        }
    }

    return result;
}

Boolean *Evaluator::nativeBoolToBooleanObject(bool input) {
    if (input) {
        return True;
    }
    return False;
}

Object *Evaluator::evalPrefixExpression(const std::string &operator_, Object &right) {
    if (operator_ == "!") {
        return this->evalBangOperatorExpression(right);
    }
    if (operator_ == "-") {
        return this->evalMinusPrefixOperatorExpression(right);
    }
    return newError("unknown operator: {}{}", operator_, right.type());
}

Object *Evaluator::evalInfixExpression(std::string &operator_, Object &left, Object &right) {
    if (left.type() == INTEGER_OBJ && right.type() == INTEGER_OBJ) {
        return this->evalIntegerInfixExpression(operator_, left, right);
    }
    if (left.type() == STRING_OBJ && right.type() == STRING_OBJ) {
        return this->evalStringInfixExpression(operator_, left, right);
    }
    if (operator_ == "==") {
        return this->nativeBoolToBooleanObject(&left == &right);
    }
    if (operator_ == "!=") {
        return this->nativeBoolToBooleanObject(&left != &right);
    }
    if (left.type() != right.type()) {
        return newError("type mismatch: {} {} {}", left.type(), operator_, right.type());
    }
    return newError("unknown operator: {} {} {}", left.type(), operator_, right.type());
}

Object *Evaluator::evalBangOperatorExpression(const Object &right) {
    if (&right == True) {
        return False;
    }
    if (&right == False) {
        return True;
    }
    if (&right == Null) {
        return True;
    }
    return False;
}

Object *Evaluator::evalMinusPrefixOperatorExpression(Object &right) {
    if (right.type() != INTEGER_OBJ) {
        return newError("unknown operator: -{}", right.type());
    }

    auto value = dynamic_cast<Integer *>(&right);
    return new Integer(-value->value);
}

Object *Evaluator::evalIntegerInfixExpression(std::string &operator_, Object &left, Object &right) {
    auto leftVal = dynamic_cast<Integer *>(&left)->value;
    auto rightVal = dynamic_cast<Integer *>(&right)->value;

    if (operator_ == "+") {
        return new Integer(leftVal + rightVal);
    }
    if (operator_ == "-") {
        return new Integer(leftVal - rightVal);
    }
    if (operator_ == "*") {
        return new Integer(leftVal * rightVal);
    }
    if (operator_ == "/") {
        return new Integer(leftVal / rightVal);
    }
    if (operator_ == "<") {
        return nativeBoolToBooleanObject(leftVal < rightVal);
    }
    if (operator_ == ">") {
        return nativeBoolToBooleanObject(leftVal > rightVal);
    }
    if (operator_ == "==") {
        return nativeBoolToBooleanObject(leftVal == rightVal);
    }
    if (operator_ == "!=") {
        return nativeBoolToBooleanObject(leftVal != rightVal);
    }
    return newError("unknown operator: {} {} {}", left.type(), operator_, right.type());
}

Object *Evaluator::evalStringInfixExpression(std::string &operator_, Object &left, Object &right) {
    if (operator_ != "+") {
        return newError("unknown operator: {} {} {}", left.type(), operator_, right.type());
    }

    const auto leftVal = dynamic_cast<String *>(&left)->value;
    const auto rightVal = dynamic_cast<String *>(&right)->value;
    return new String(leftVal + rightVal);
}

Object *Evaluator::evalIfExpression(Ast::IfExpression &ie, Environment &env) {
    const auto condition = this->Eval(*ie.condition, env);
    if (isError(condition)) {
        return condition;
    }

    if (isTruthy(*condition)) {
        return this->Eval(*ie.consequence, env);
    }
    if (ie.alternative != nullptr) {
        return this->Eval(*ie.alternative, env);
    }
    return Null;
}

Object *Evaluator::evalIdentifier(Ast::Identifier &node, Environment &env) {
    if (auto [val,ok] = env.get(node.value); ok) {
        return val;
    }

    if (builtins.find(node.value) != builtins.end()) {
        return builtins[node.value];
    }

    return newError("identifier not found: " + node.value);
}

bool Evaluator::isTruthy(Object &obj) {
    if (&obj == Null) {
        return false;
    }
    if (&obj == True) {
        return true;
    }
    if (&obj == False) {
        return false;
    }
    return true;
}

template<typename... Args>
Error *Evaluator::newError(const std::string &format, Args... args) {
    return new Error(fmt::format(format, args...));
}

bool Evaluator::isError(Object *obj) {
    if (obj != nullptr) {
        return obj->type() == ERROR_OBJ;
    }
    return false;
}

std::vector<Object *> Evaluator::evalExpressions(std::vector<Ast::Expression *> &expressions, Environment &env) {
    std::vector<Object *> result{};

    for (auto *expression: expressions) {
        auto evaluated = this->Eval(*expression, env);
        if (isError(evaluated)) {
            return {evaluated};
        }
        result.emplace_back(evaluated);
    }
    return result;
}

Object *Evaluator::applyFunction(Object &_fn, std::vector<Object *> &args) {
    if (instance_of<Object, Function>(_fn)) {
        const auto fn = dynamic_cast<Function *>(&_fn);
        const auto extendedEnv = this->extendFunctionEnv(*fn, args);
        const auto evaluated = this->Eval(*fn->body, *extendedEnv);
        return this->unwrapReturnValue(*evaluated);
    }
    if (instance_of<Object, Builtin>(_fn)) {
        const auto fn = dynamic_cast<Builtin *>(&_fn);
        if (const auto result = fn->fn(args); result != nullptr) {
            return result;
        }
        return Null;
    }
    return nullptr;
}

Environment *Evaluator::extendFunctionEnv(const Function &fn, const std::vector<Object *> &args) {
    const auto env = new Environment(fn.env);
    for (auto i = 0; i < fn.parameters.size(); ++i) {
        env->set(fn.parameters[i].get()->value, *args[i]);
    }
    return env;
}

Object *Evaluator::unwrapReturnValue(Object &obj) {
    if (instance_of<Object, ReturnValue>(obj)) {
        return dynamic_cast<ReturnValue *>(&obj)->value;
    }
    return &obj;
}

Object *Evaluator::evalIndexExpression(Object &left, Object &index) {
    if (left.type() == ARRAY_OBJ && index.type() == INTEGER_OBJ) {
        return this->evalArrayIndexExpression(left, index);
    }
    if (left.type() == HASH_OBJ) {
        return this->evalHashIndexExpression(left, index);
    }
    return newError("index operator not supported: {}", left.type());
}

Object *Evaluator::evalArrayIndexExpression(Object &array, Object &index) {
    auto arrayObject = dynamic_cast<Array *>(&array);
    auto idx = dynamic_cast<Integer *>(&index)->value;

    if (idx < 0 || idx > static_cast<int64_t>(arrayObject->elements.size()) - 1) {
        return Null;
    }

    return arrayObject->elements[idx];
}

Object *Evaluator::evalHashLiteral(Ast::HashLiteral &node, Environment &env) {
    std::unordered_map<HashKey, HashPair> pairs{};

    for (auto &[_,pair]: node.pairs) {
        auto key = this->Eval(*pair.first.get(), env);
        if (isError(key)) {
            return key;
        }
        auto hashKey = dynamic_cast<Hashable *>(key);
        if (hashKey == nullptr) {
            return newError("unusable as hash key: {}", key->type());
        }

        auto value = this->Eval(*pair.second.get(), env);
        if (isError(value)) {
            return value;
        }

        auto hashed = hashKey->hash_key();
        pairs.emplace(hashed, *new HashPair(*key, *value));
    }
    return new Hash(pairs);
}

Object *Evaluator::evalHashIndexExpression(Object &hash, Object &index) {
    auto hashObject = dynamic_cast<Hash *>(&hash);
    auto key = dynamic_cast<Hashable *>(&index);
    if (key == nullptr) {
        return newError("unusable as hash key: {}", index.type());
    }

    auto hash_key = key->hash_key();
    if (hashObject->pairs.find(key->hash_key()) == hashObject->pairs.end()) {
        return Null;
    }

    return hashObject->pairs[key->hash_key()].value;
}
