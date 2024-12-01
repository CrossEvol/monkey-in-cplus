//
// Created by mizuk on 2024/11/27.
//

#include "object.h"

#include "fmt/format.h"

ObjectType Integer::inspect() {
    return fmt::format("{}", value);
}

ObjectType Integer::type() {
    return INTEGER_OBJ;
}

HashKey Integer::hash_key() {
    return {this->type(), static_cast<uint64_t>(this->value)};
}

ObjectType Boolean::type() {
    return BOOLEAN_OBJ;
}

std::string Boolean::inspect() {
    return value ? "true" : "false";
}

HashKey Boolean::hash_key() {
    return {this->type(), value ? 1ULL : 0ULL};
}

ObjectType Null::type() {
    return NULL_OBJ;
}

std::string Null::inspect() {
    return "null";
}

ObjectType ReturnValue::type() {
    return RETURN_VALUE_OBJ;
}

std::string ReturnValue::inspect() {
    return value->inspect();
}

ObjectType Error::type() {
    return ERROR_OBJ;
}

std::string Error::inspect() {
    return "ERROR: " + message;
}

ObjectType Function::type() {
    return FUNCTION_OBJ;
}

std::string Function::inspect() {
    std::vector<std::string> params;
    for (const auto &p: parameters) {
        params.push_back(p->string());
    }

    return fmt::format("fn({}) {{\n{}\n}}",
                       fmt::join(params, ", "),
                       body->string());
}

ObjectType String::type() {
    return STRING_OBJ;
}

std::string String::inspect() {
    return value;
}

HashKey String::hash_key() {
    // Using std::hash for string hashing
    constexpr std::hash<std::string> std_hash;
    return {this->type(), std_hash(value)};
}

ObjectType Builtin::type() {
    return BUILTIN_OBJ;
}

std::string Builtin::inspect() {
    return "builtin function";
}

ObjectType Array::type() {
    return ARRAY_OBJ;
}

std::string Array::inspect() {
    std::vector<std::string> elements_str;
    for (const auto &elem: elements) {
        elements_str.push_back(elem->inspect());
    }

    return fmt::format("[{}]", fmt::join(elements_str, ", "));
}

ObjectType Hash::type() {
    return HASH_OBJ;
}

std::string Hash::inspect() {
    std::vector<std::string> pairs_str;
    for (const auto &[_, pair]: pairs) {
        pairs_str.push_back(
            fmt::format("{}: {}",
                        pair.key->inspect(),
                        pair.value->inspect()));
    }

    return fmt::format("{{{}}}", fmt::join(pairs_str, ", "));
}

ObjectType CompiledFunction::type() {
    return COMPILED_FUNCTION_OBJ;
}

std::string CompiledFunction::inspect() {
    return fmt::format("CompiledFunction[{:p}]", static_cast<void *>(this));
}

ObjectType Closure::type() {
    return CLOSURE_OBJ;
}

std::string Closure::inspect() {
    return fmt::format("Closure[{:p}]", static_cast<void *>(this));
}
