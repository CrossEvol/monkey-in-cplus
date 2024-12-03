//
// Created by mizuk on 2024/12/3.
//

#include "frame.h"

Instructions Frame::instructions() const {
    return this->cl->fn.instructions;
}
