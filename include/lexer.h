#pragma once

#include "types.h"
#include "token.h"
#include "source.h"

typedef struct mtlexer mtlexer;
struct mtlexer {
  source_t*   src;
  size_t      position;
  size_t      length;
};

mtlexer*    lexer_new(char* path);
void        lexer_free(mtlexer* lx);
token_t*    lexer_lex(mtlexer* lx);
