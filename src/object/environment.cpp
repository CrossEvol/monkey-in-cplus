//
// Created by mizuk on 2024/11/27.
//

#include "environment.h"

#include <stdexcept>

#include "fmt/format.h"

Object* Environment::set(const std::string &name, Object &val) {
    this->store[name] = std::make_shared<Object *>(&val);
    return &val;
}

std::pair<Object *, bool> Environment::get(const std::string &name) {
    if (const auto iterator = this->store.find(name); iterator != this->store.end()) {
        return {*iterator->second, true};
    }
    if (this->outer != nullptr) {
        return this->outer->get(name);
    }
    return {{}, false};
}
