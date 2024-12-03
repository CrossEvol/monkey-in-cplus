//
// Created by mizuk on 2024/12/3.
//

#ifndef FRAME_H
#define FRAME_H

#include "../object/object.h"

struct Frame {
    Closure *cl;
    int ip;
    int basePointer;

    Frame(Closure &closure, const int base_pointer)
        : cl(&closure),
          ip(-1), basePointer(base_pointer) {
    }

    Instructions instructions() const;
};


#endif //FRAME_H
