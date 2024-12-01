#include <catch2/catch_test_macros.hpp>
#define CATCH_CONFIG_MAIN

#include "../src/compiler/symbol_table.h"

TEST_CASE("Test Define", "[symbol_table]") {
    std::map<std::string, Symbol> expected = {
        {"a", Symbol("a", GlobalScope, 0)},
        {"b", Symbol("b", GlobalScope, 1)},
        {"c", Symbol("c", LocalScope, 0)},
        {"d", Symbol("d", LocalScope, 1)},
        {"e", Symbol("e", LocalScope, 0)},
        {"f", Symbol("f", LocalScope, 1)}
    };

    auto global = std::make_shared<SymbolTable>();

    auto a = global->define("a");
    INFO("expected a=" << expected["a"].name << "," << expected["a"].scope << "," << expected["a"].index
        << " got=" << a.name << "," << a.scope << "," << a.index);
    REQUIRE(a.name == expected["a"].name);
    REQUIRE(a.scope == expected["a"].scope);
    REQUIRE(a.index == expected["a"].index);

    auto b = global->define("b");
    INFO("expected b=" << expected["b"].name << "," << expected["b"].scope << "," << expected["b"].index
        << " got=" << b.name << "," << b.scope << "," << b.index);
    REQUIRE(b.name == expected["b"].name);
    REQUIRE(b.scope == expected["b"].scope);
    REQUIRE(b.index == expected["b"].index);

    auto firstLocal = std::make_shared<SymbolTable>(global);

    auto c = firstLocal->define("c");
    INFO("expected c=" << expected["c"].name << "," << expected["c"].scope << "," << expected["c"].index
        << " got=" << c.name << "," << c.scope << "," << c.index);
    REQUIRE(c.name == expected["c"].name);
    REQUIRE(c.scope == expected["c"].scope);
    REQUIRE(c.index == expected["c"].index);

    auto d = firstLocal->define("d");
    INFO("expected d=" << expected["d"].name << "," << expected["d"].scope << "," << expected["d"].index
        << " got=" << d.name << "," << d.scope << "," << d.index);
    REQUIRE(d.name == expected["d"].name);
    REQUIRE(d.scope == expected["d"].scope);
    REQUIRE(d.index == expected["d"].index);

    auto secondLocal = std::make_shared<SymbolTable>(firstLocal);

    auto e = secondLocal->define("e");
    INFO("expected e=" << expected["e"].name << "," << expected["e"].scope << "," << expected["e"].index
        << " got=" << e.name << "," << e.scope << "," << e.index);
    REQUIRE(e.name == expected["e"].name);
    REQUIRE(e.scope == expected["e"].scope);
    REQUIRE(e.index == expected["e"].index);

    auto f = secondLocal->define("f");
    INFO("expected f=" << expected["f"].name << "," << expected["f"].scope << "," << expected["f"].index
        << " got=" << f.name << "," << f.scope << "," << f.index);
    REQUIRE(f.name == expected["f"].name);
    REQUIRE(f.scope == expected["f"].scope);
    REQUIRE(f.index == expected["f"].index);
}

TEST_CASE("Test Resolve Global", "[symbol_table]") {
    auto global = std::make_shared<SymbolTable>();
    global->define("a");
    global->define("b");

    std::vector<Symbol> expected = {
        Symbol("a", GlobalScope, 0),
        Symbol("b", GlobalScope, 1)
    };

    for (const auto &sym: expected) {
        auto [result, ok] = global->resolve(sym.name);
        INFO("name " << sym.name << " not resolvable");
        REQUIRE(ok);

        INFO("expected " << sym.name << " to resolve to " << sym.name << "," << sym.scope << "," << sym.index
            << " got=" << result.name << "," << result.scope << "," << result.index);
        REQUIRE(result.name == sym.name);
        REQUIRE(result.scope == sym.scope);
        REQUIRE(result.index == sym.index);
    }
}

TEST_CASE("Test Resolve Local", "[symbol_table]") {
    auto global = std::make_shared<SymbolTable>();
    global->define("a");
    global->define("b");

    auto local = std::make_shared<SymbolTable>(global);
    local->define("c");
    local->define("d");

    std::vector<Symbol> expected = {
        Symbol("a", GlobalScope, 0),
        Symbol("b", GlobalScope, 1),
        Symbol("c", LocalScope, 0),
        Symbol("d", LocalScope, 1)
    };

    for (const auto &sym: expected) {
        auto [result, ok] = local->resolve(sym.name);
        INFO("name " << sym.name << " not resolvable");
        REQUIRE(ok);

        INFO("expected " << sym.name << " to resolve to " << sym.name << "," << sym.scope << "," << sym.index
            << " got=" << result.name << "," << result.scope << "," << result.index);
        REQUIRE(result.name == sym.name);
        REQUIRE(result.scope == sym.scope);
        REQUIRE(result.index == sym.index);
    }
}

TEST_CASE("Test Define Resolve Builtins", "[symbol_table]") {
    auto global = std::make_shared<SymbolTable>();
    auto firstLocal = std::make_shared<SymbolTable>(global);
    auto secondLocal = std::make_shared<SymbolTable>(firstLocal);

    std::vector<Symbol> expected = {
        Symbol("a", BuiltinScope, 0),
        Symbol("c", BuiltinScope, 1),
        Symbol("e", BuiltinScope, 2),
        Symbol("f", BuiltinScope, 3)
    };

    for (int i = 0; i < expected.size(); i++) {
        global->defineBuiltin(i, expected[i].name);
    }

    std::vector<std::shared_ptr<SymbolTable> > tables = {global, firstLocal, secondLocal};
    for (const auto &table: tables) {
        for (const auto &sym: expected) {
            auto [result, ok] = table->resolve(sym.name);
            INFO("name " << sym.name << " not resolvable");
            REQUIRE(ok);

            INFO("expected " << sym.name << " to resolve to " << sym.name << "," << sym.scope << "," << sym.index
                << " got=" << result.name << "," << result.scope << "," << result.index);
            REQUIRE(result.name == sym.name);
            REQUIRE(result.scope == sym.scope);
            REQUIRE(result.index == sym.index);
        }
    }
}

TEST_CASE("Test Resolve Free", "[symbol_table]") {
    auto global = std::make_shared<SymbolTable>();
    global->define("a");
    global->define("b");

    auto firstLocal = std::make_shared<SymbolTable>(global);
    firstLocal->define("c");
    firstLocal->define("d");

    auto secondLocal = std::make_shared<SymbolTable>(firstLocal);
    secondLocal->define("e");
    secondLocal->define("f");

    struct TestCase {
        std::shared_ptr<SymbolTable> table;
        std::vector<Symbol> expectedSymbols;
        std::vector<Symbol> expectedFreeSymbols;
    };

    std::vector<TestCase> tests = {
        {
            firstLocal,
            {
                Symbol("a", GlobalScope, 0),
                Symbol("b", GlobalScope, 1),
                Symbol("c", LocalScope, 0),
                Symbol("d", LocalScope, 1)
            },
            {}
        },
        {
            secondLocal,
            {
                Symbol("a", GlobalScope, 0),
                Symbol("b", GlobalScope, 1),
                Symbol("c", FreeScope, 0),
                Symbol("d", FreeScope, 1),
                Symbol("e", LocalScope, 0),
                Symbol("f", LocalScope, 1)
            },
            {
                Symbol("c", LocalScope, 0),
                Symbol("d", LocalScope, 1)
            }
        }
    };

    for (const auto &tt: tests) {
        for (const auto &sym: tt.expectedSymbols) {
            auto [result, ok] = tt.table->resolve(sym.name);
            INFO("name " << sym.name << " not resolvable");
            REQUIRE(ok);

            INFO("expected " << sym.name << " to resolve to " << sym.name << "," << sym.scope << "," << sym.index
                << " got=" << result.name << "," << result.scope << "," << result.index);
            REQUIRE(result.name == sym.name);
            REQUIRE(result.scope == sym.scope);
            REQUIRE(result.index == sym.index);
        }

        INFO("wrong number of free symbols. got=" << tt.table->free_symbols.size()
            << ", want=" << tt.expectedFreeSymbols.size());
        REQUIRE(tt.table->free_symbols.size() == tt.expectedFreeSymbols.size());

        for (size_t i = 0; i < tt.expectedFreeSymbols.size(); i++) {
            const auto &expected = tt.expectedFreeSymbols[i];
            const auto &result = tt.table->free_symbols[i];

            INFO("wrong free symbol. got=" << result.name << "," << result.scope << "," << result.index
                << " want=" << expected.name << "," << expected.scope << "," << expected.index);
            REQUIRE(result.name == expected.name);
            REQUIRE(result.scope == expected.scope);
            REQUIRE(result.index == expected.index);
        }
    }
}

TEST_CASE("Test Resolve Unresolvable Free", "[symbol_table]") {
    auto global = std::make_shared<SymbolTable>();
    global->define("a");

    auto firstLocal = std::make_shared<SymbolTable>(global);
    firstLocal->define("c");

    auto secondLocal = std::make_shared<SymbolTable>(firstLocal);
    secondLocal->define("e");
    secondLocal->define("f");

    std::vector<Symbol> expected = {
        Symbol("a", GlobalScope, 0),
        Symbol("c", FreeScope, 0),
        Symbol("e", LocalScope, 0),
        Symbol("f", LocalScope, 1)
    };

    for (const auto &sym: expected) {
        auto [result, ok] = secondLocal->resolve(sym.name);
        INFO("name " << sym.name << " not resolvable");
        REQUIRE(ok);

        INFO("expected " << sym.name << " to resolve to " << sym.name << "," << sym.scope << "," << sym.index
            << " got=" << result.name << "," << result.scope << "," << result.index);
        REQUIRE(result.name == sym.name);
        REQUIRE(result.scope == sym.scope);
        REQUIRE(result.index == sym.index);
    }

    std::vector<std::string> expectedUnresolvable = {"b", "d"};

    for (const auto &name: expectedUnresolvable) {
        auto [_, ok] = secondLocal->resolve(name);
        INFO("name " << name << " resolved, but was expected not to");
        REQUIRE_FALSE(ok);
    }
}
