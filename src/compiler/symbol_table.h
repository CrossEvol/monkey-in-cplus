//
// Created by mizuk on 2024/11/29.
//

#ifndef SYMBOL_TABLE_H
#define SYMBOL_TABLE_H
#include <map>
#include <memory>
#include <string>
#include <vector>

using SymbolScope = std::string;

inline SymbolScope LocalScope = "LOCAL";
inline SymbolScope GlobalScope = "GLOBAL";
inline SymbolScope BuiltinScope = "BUILTIN";
inline SymbolScope FreeScope = "FREE";

struct Symbol {
    std::string name;
    SymbolScope scope;
    int index;

    Symbol() = default;

    Symbol(const std::string &name, const SymbolScope &scope, int index)
        : name(name),
          scope(scope),
          index(index) {
    }
};

class SymbolTable;

class SymbolTable {
public:
    std::shared_ptr<SymbolTable> outer;
    std::map<std::string, Symbol> store;
    int num_definitions;
    std::vector<Symbol> free_symbols;

    SymbolTable() = default;

    explicit SymbolTable(const std::shared_ptr<SymbolTable> &outer)
        : outer(outer), num_definitions(0) {
    }

    Symbol define(const std::string &name);

    std::pair<Symbol, bool> resolve(const std::string &name);

    Symbol defineBuiltin(int index, const std::string &name);

    Symbol defineFree(const Symbol &original);
};

#endif //SYMBOL_TABLE_H
