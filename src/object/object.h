//
// Created by mizuk on 2024/11/27.
//

#ifndef OBJECT_H
#define OBJECT_H
#include <string>
#include <utility>
#include <vector>
#include <memory>
#include <functional>
#include <unordered_map>
#include "../ast/ast.h"
#include "../code/code.h"

using ObjectType = std::string;

inline ObjectType NULL_OBJ = "NULL";
inline ObjectType ERROR_OBJ = "ERROR";

inline ObjectType INTEGER_OBJ = "INTEGER";
inline ObjectType BOOLEAN_OBJ = "BOOLEAN";
inline ObjectType STRING_OBJ = "STRING";

inline ObjectType RETURN_VALUE_OBJ = "RETURN_VALUE";

inline ObjectType FUNCTION_OBJ = "FUNCTION";
inline ObjectType BUILTIN_OBJ = "BUILTIN";
inline ObjectType COMPILED_FUNCTION_OBJ = "COMPILED_FUNCTION_OBJ";
inline ObjectType CLOSURE_OBJ = "CLOSURE";

inline ObjectType ARRAY_OBJ = "ARRAY";
inline ObjectType HASH_OBJ = "HASH";

class Object;

struct HashKey {
    ObjectType type;
    uint64_t value;

    bool operator==(const HashKey &other) const {
        return type == other.type && value == other.value;
    }
};

namespace std {
    template<>
    struct hash<HashKey> {
        size_t operator()(const HashKey &k) const noexcept {
            size_t h1 = std::hash<std::string>{}(k.type);
            size_t h2 = std::hash<uint64_t>{}(k.value);
            return h1 ^ (h2 << 1);
        }
    };
}

class Hashable {
public:
    virtual ~Hashable() = default;

    virtual HashKey hash_key() = 0;
};

class Object {
public:
    virtual ~Object() = default;

    virtual ObjectType type() = 0;

    virtual std::string inspect() = 0;
};

class Integer final : public Object, public Hashable {
public:
    int64_t value;

    explicit Integer(const int64_t value)
        : value(value) {
    }

    ~Integer() override = default;

    ObjectType inspect() override;

    ObjectType type() override;

    HashKey hash_key() override;
};

class Boolean final : public Object, public Hashable {
public:
    bool value;

    explicit Boolean(const bool value) : value(value) {
    }

    ~Boolean() override = default;

    ObjectType type() override;

    std::string inspect() override;

    HashKey hash_key() override;
};

namespace OBJ {
    class Null final : public Object {
    public:
        Null() = default;

        ~Null() override = default;

        ObjectType type() override;

        std::string inspect() override;
    };
}

class ReturnValue final : public Object {
public:
    std::shared_ptr<Object> value;

    explicit ReturnValue(std::shared_ptr<Object> value) : value(std::move(value)) {
    }

    ~ReturnValue() override = default;

    ObjectType type() override;

    std::string inspect() override;
};

class Error final : public Object {
public:
    std::string message;

    explicit Error(std::string message) : message(std::move(message)) {
    }

    ~Error() override = default;

    ObjectType type() override;

    std::string inspect() override;
};

class String final : public Object, public Hashable {
public:
    std::string value;

    explicit String(std::string value) : value(std::move(value)) {
    }

    ~String() override = default;

    ObjectType type() override;

    std::string inspect() override;

    HashKey hash_key() override;
};

class Environment;

class Function final : public Object {
public:
    std::vector<std::shared_ptr<Ast::Identifier> > parameters;
    std::shared_ptr<Ast::BlockStatement> body;
    std::shared_ptr<Environment> env;

    Function(std::vector<std::shared_ptr<Ast::Identifier> > params,
             std::shared_ptr<Ast::BlockStatement> body,
             std::shared_ptr<Environment> env)
        : parameters(std::move(params))
          , body(std::move(body))
          , env(std::move(env)) {
    }

    ~Function() override = default;

    ObjectType type() override;

    std::string inspect() override;
};

// You'll need to implement these types based on your needs
using BuiltinFunction = Object*(*)(const std::vector<Object *> &);

class Builtin final : public Object {
public:
    BuiltinFunction fn;

    explicit Builtin(const BuiltinFunction fn) : fn(fn) {
    }

    ~Builtin() override = default;

    ObjectType type() override;

    std::string inspect() override;
};

class Array final : public Object {
public:
    std::vector<Object *> elements;

    explicit Array(std::vector<Object *> elements)
        : elements(std::move(elements)) {
    }

    ~Array() override = default;

    ObjectType type() override;

    std::string inspect() override;
};

struct HashPair {
    Object *key;
    Object *value;

    HashPair() = default;

    HashPair(Object &key, Object &value)
        : key(&key),
          value(&value) {
    }
};

class Hash final : public Object {
public:
    std::unordered_map<HashKey, HashPair> pairs;

    explicit Hash(const std::unordered_map<HashKey, HashPair> &pairs = {})
        : pairs(pairs) {
    }

    ~Hash() override = default;

    ObjectType type() override;

    std::string inspect() override;
};

class CompiledFunction final : public Object {
public:
    Instructions instructions;
    int numLocals;
    int numParameters;

    explicit CompiledFunction(const Instructions &instructions)
        : instructions(instructions), numLocals(0), numParameters(0) {
    }

    CompiledFunction(Instructions instructions, const int num_locals, const int num_parameters)
        : instructions(std::move(instructions)),
          numLocals(num_locals),
          numParameters(num_parameters) {
    }

    ~CompiledFunction() override = default;

    ObjectType type() override;

    std::string inspect() override;
};

class Closure final : public Object {
public:
    CompiledFunction fn;
    std::vector<Object *> free{};

    explicit Closure(const CompiledFunction &fn)
        : fn(fn) {
    }

    Closure(const CompiledFunction &fn, const std::vector<Object *> &free)
        : fn(fn),
          free(free) {
    }

    ~Closure() override = default;

    ObjectType type() override;

    std::string inspect() override;
};

#endif //OBJECT_H
