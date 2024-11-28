//
// Created by mizuk on 2024/11/28.
//

#ifndef PARSER_TRACING_H
#define PARSER_TRACING_H
#include <string>

static int traceLevel = 0;

inline std::string traceIdentPlaceholder = "\t";

std::string identLevel();

void tracePrint(const std::string &fs);

void incIdent();

void decIdent();

std::string trace(std::string msg);

void untrace(const std::string &msg);

#endif //PARSER_TRACING_H
