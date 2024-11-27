//
// Created by mizuk on 2024/11/27.
//

#include  "token.h"

TokenType lookupIdent(std::string_view ident) {
    return keywords.count(std::string(ident)) > 0 ? keywords[std::string(ident)] : IDENT;
}
