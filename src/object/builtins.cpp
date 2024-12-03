//
// Created by mizuk on 2024/11/27.
//

#include "builtins.h"

#include <array>
#include <complex>

#include "../common/common.h"
#include "fmt/format.h"

Object *monkey_len(const std::vector<Object *> &args) {
    if (args.size() != 1) {
        return newError("wrong number of arguments. got={:d}, want=1",
                        args.size());
    }
    if (auto* str = dynamic_cast<String*>(args[0])) {
        return new Integer(static_cast<int64_t>(str->value.size()));
    }
    if (auto* array = dynamic_cast<Array*>(args[0])) {
        return new Integer(static_cast<int64_t>(array->elements.size()));
    }
    return newError("argument to `len` not supported, got {:s}",
                    args[0]->type());
}

Object *monkey_puts(const std::vector<Object *> &args) {
    for (const auto &arg: args) {
        fmt::println(arg->inspect());
    }
    return nullptr;
}

Object *monkey_first(const std::vector<Object *> &args) {
    if (args.size() != 1) {
        return new Error("wrong number of arguments. got=" + std::to_string(args.size()) + ", want=1");
    }

    if (auto *array = dynamic_cast<Array *>(args[0])) {
        if (array->elements.empty()) {
            return nullptr;
        }
        return array->elements[0];
    }
    return newError("argument to `first` must be ARRAY, got {:s}",
                    args[0]->type());
}

Object *monkey_last(const std::vector<Object *> &args) {
    if (args.size() != 1) {
        return newError("wrong number of arguments. got={:d}, want=1",
                        args.size());
    }
    if (args[0]->type() != ARRAY_OBJ) {
        return newError("argument to `last` must be ARRAY, got {:s}",
                        args[0]->type());
    }
    auto *array = dynamic_cast<Array *>(args[0]);
    if (!array->elements.empty()) {
        return array->elements.back();
    }
    return nullptr;
}

Object *monkey_rest(const std::vector<Object *> &args) {
    if (args.size() != 1) {
        return newError("wrong number of arguments. got={:d}, want=1",
                        args.size());
    }
    if (args[0]->type() != ARRAY_OBJ) {
        return newError("argument to `rest` must be ARRAY, got {:s}",
                        args[0]->type());
    }
    auto *array = dynamic_cast<Array *>(args[0]);
    if (!array->elements.empty()) {
        const std::vector elements(array->elements.begin() + 1, array->elements.end());
        return new Array(elements);
    }
    return nullptr;
}

Object *monkey_push(const std::vector<Object *> &args) {
    if (args.size() != 2) {
        return newError("wrong number of arguments. got={:d}, want=2",
                        args.size());
    }
    if (args[0]->type() != ARRAY_OBJ) {
        return newError("argument to `push` must be ARRAY, got {:s}",
                        args[0]->type());
    }
    auto *array = dynamic_cast<Array *>(args[0]);
    auto elements = array->elements;
    elements.push_back(args[1]);
    return new Array(elements);
}

template<typename... Args>
Error *newError(const std::string &format, Args &&... args) {
    return new Error{fmt::format(format, std::forward<Args>(args)...)};
}

Builtin *getBuiltinByName(const std::string &name) {
    for (auto [fst,snd]: builtins) {
        if (fst == name) {
            return snd;
        }
    }
    return nullptr;
}
