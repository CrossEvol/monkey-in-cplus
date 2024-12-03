//
// Created by mizuk on 2024/11/27.
//

#ifndef BUILTINS_H
#define BUILTINS_H
#include <any>
#include <map>
#include <memory>
#include <string>

#include "object.h"

Object *monkey_len(const std::vector<Object*>& args);

Object *monkey_puts(const std::vector<Object* > &args);

Object *monkey_first(const std::vector<Object*>& args);

Object *monkey_last(const std::vector<Object*>& args);

Object *monkey_rest(const std::vector<Object*>& args);

Object *monkey_push(const std::vector<Object*>& args);

inline std::vector<std::pair<std::string, Builtin *> > builtins = {
    {"len", new Builtin(&monkey_len)},
    {"puts", new Builtin(&monkey_puts)},
    {"first", new Builtin(&monkey_first)},
    {"last", new Builtin(&monkey_last)},
    {"rest", new Builtin(&monkey_rest)},
    {"push", new Builtin(&monkey_push)},
};

template<typename... Args>
Error *newError(const std::string &format, Args &&... args);

Builtin *getBuiltinByName(const std::string &name);

#endif //BUILTINS_H
