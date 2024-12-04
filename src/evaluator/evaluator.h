//
// Created by mizuk on 2024/12/4.
//

#ifndef EVALUATOR_H
#define EVALUATOR_H
#include <map>
#include <string>

#include "../object/environment.h"
#include "../object/object.h"

class Evaluator {
public:
    static std::map<std::string, Builtin *> builtins;
    static Boolean *True;
    static Boolean *False;
    static OBJ::Null *Null;

    Object *Eval(Ast::Node &_node, Environment &env);

private:
    Object *evalProgram(Ast::Program &program, Environment &env);

    Object *evalBlockStatement(Ast::BlockStatement &block, Environment &env);

    Boolean *nativeBoolToBooleanObject(bool input);

    Object *evalPrefixExpression(const std::string &operator_, Object &right);

    Object *evalInfixExpression(std::string &operator_, Object &left, Object &right);

    Object *evalBangOperatorExpression(const Object &right);

    Object *evalMinusPrefixOperatorExpression(Object &right);

    Object *evalIntegerInfixExpression(std::string &operator_, Object &left, Object &right);

    Object *evalStringInfixExpression(std::string &operator_, Object &left, Object &right);

    Object *evalIfExpression(Ast::IfExpression &ie, Environment &env);

    Object *evalIdentifier(Ast::Identifier &node, Environment &env);

    bool isTruthy(Object &obj);

    template<typename... Args>
    Error *newError(const std::string &format, Args... args);

    bool isError(Object *obj);

    std::vector<Object *> evalExpressions(std::vector<Ast::Expression *> &expressions, Environment &env);

    Object *applyFunction(Object &_fn, std::vector<Object *> &args);

    Environment *extendFunctionEnv(const Function &fn, const std::vector<Object *> &args);

    Object *unwrapReturnValue(Object &obj);

    Object *evalIndexExpression(Object &left, Object &index);

    Object *evalArrayIndexExpression(Object &array, Object &index);

    Object *evalHashLiteral(Ast::HashLiteral &node, Environment &env);

    Object *evalHashIndexExpression(Object &hash, Object &index);
};

#endif //EVALUATOR_H
