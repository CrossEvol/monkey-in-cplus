//
// Created by mizuk on 2024/12/3.
//

#ifndef COMMON_SUITE_H
#define COMMON_SUITE_H

template<class... Ts>
struct overloaded : Ts... {
    using Ts::operator()...;
};

template<class... Ts>
overloaded(Ts...) -> overloaded<Ts...>;

#endif //COMMON_SUITE_H
