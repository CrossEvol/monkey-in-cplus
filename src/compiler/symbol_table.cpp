//
// Created by mizuk on 2024/11/29.
//

#include "symbol_table.h"

Symbol SymbolTable::define(const std::string &name) {
    const auto scope = this->outer == nullptr ? GlobalScope : LocalScope;
    Symbol symbol{name, scope, num_definitions};
    this->store[name] = symbol;
    this->num_definitions++;
    return symbol;
}

std::pair<Symbol, bool> SymbolTable::resolve(const std::string &name) {
    auto ok = this->store.count(name) > 0;
    if (!ok && this->outer != nullptr) {
        auto [obj,resolved] = this->outer->resolve(name);
        if (!resolved) {
            return {{}, resolved};
        }

        if (obj.scope == GlobalScope || obj.scope == BuiltinScope) {
            return {obj, resolved};
        }

        auto freeSymbol = this->defineFree(obj);
        return {freeSymbol, true};
    }
    return {ok ? this->store[name] : Symbol{}, ok};
}

Symbol SymbolTable::defineBuiltin(const int index, const std::string &name) {
    Symbol symbol{name, BuiltinScope, index};
    this->store[name] = symbol;
    return symbol;
}

Symbol SymbolTable::defineFree(const Symbol &original) {
    this->free_symbols.push_back(original);

    Symbol symbol{original.name, FreeScope, static_cast<int>(this->free_symbols.size() - 1)};
    this->store[original.name] = symbol;

    return symbol;
}
