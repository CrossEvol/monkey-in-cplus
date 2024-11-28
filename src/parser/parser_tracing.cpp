//
// Created by mizuk on 2024/11/28.
//

#include "parser_tracing.h"
#include <sstream>

#include "fmt/printf.h"

std::string identLevel() {
    std::stringstream oss;
    for (auto i = 0; i < traceLevel - 1; i++) {
        oss << traceIdentPlaceholder;
    }
    return oss.str();
}

void tracePrint(const std::string &fs) {
    fmt::printf("%s%s\n", identLevel(), fs);
}

void incIdent() {
    traceLevel++;
}

void decIdent() {
    traceLevel--;
}

std::string trace(std::string msg) {
    incIdent();
    tracePrint("BEGIN " + msg);
    return msg;
}

void untrace(const std::string &msg) {
    tracePrint("END " + msg);
    decIdent();
}
