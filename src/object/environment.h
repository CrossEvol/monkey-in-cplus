//
// Created by mizuk on 2024/11/27.
//

#ifndef ENVIRONMENT_H
#define ENVIRONMENT_H
#include <map>
#include <memory>
#include <string>

#include "object.h"

class Environment {
private:
    std::map<std::string, std::shared_ptr<Object*> > store;
    std::shared_ptr<Environment> outer;

public:
    Environment() {
        store = {};
        outer = nullptr;
    }

    explicit Environment(const std::shared_ptr<Environment> &outer)
        : Environment() {
        this->outer = outer;
    }

    Object* set(const std::string &name, Object &val);

    std::pair<Object*, bool> get(const std::string &name);
};

#endif //ENVIRONMENT_H
