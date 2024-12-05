//
// Created by mizuk on 2024/12/5.
//

#ifndef REPL_H
#define REPL_H

#include <string>
#include <vector>
#include <iostream>

namespace Repl {

constexpr auto PROMPT = ">> ";

constexpr auto MONKEY_FACE = R"(            __,__
   .--.  .-"     "-.  .--.
  / .. \/  .-. .-.  \/ .. \
 | |  '|  /   Y   \  |'  | |
 | \   \  \ 0 | 0 /  /   / |
  \ '- ,\.-"""""""-./, -' /
   ''-' /_   ^ ^   _\ '-''
       |  \._   _./  |
       \   \ '~' /   /
        '._ '-=-' _.'
           '-----'
)";

void start(std::istream& in, std::ostream& out);
void printParserErrors(std::ostream& out, const std::vector<std::string>& errors);

}

#endif //REPL_H
