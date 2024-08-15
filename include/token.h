#pragma once

#include "types.h"
#include "source.h"

typedef u16   token_kind_t;

enum {
  TOK_INT,
  TOK_FLOAT,
  TOK_CHAR,
  TOK_STRING,
  TOK_KEYWORD,
  TOK_IDENTIFIER,
  TOK_PUNCTUATER,
  TOK_INDENT,
  TOK_END
};

typedef struct token_t token_t;
struct token_t {
  token_kind_t  kind;
  token_t*      prev;
  token_t*      next;
  char*         str;
  size_t        len;
  size_t        pos;
  source_t*     src;

};

token_t*
token_new(token_kind_t kind, token_t* prev, char* str, size_t len, size_t pos);
