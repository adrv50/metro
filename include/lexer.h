#pragma once

#include "types.h"
#include "token.h"
#include "source.h"

typedef struct {
  source_file* src;
  size_t position;
  size_t length;
} mtlexer;

mtlexer* lexer_new(source_file* src);
void lexer_free(mtlexer* lx);
mt_token* lexer_lex(mtlexer* lx);
