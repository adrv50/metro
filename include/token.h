#pragma once

#include "types.h"
#include "source.h"
#include "object.h"

typedef enum {
  TOK_INT,
  TOK_FLOAT,
  TOK_CHAR,
  TOK_STRING,
  TOK_KEYWORD,
  TOK_IDENTIFIER,
  TOK_PUNCTUATER,
  TOK_END
} token_kind_t;

typedef struct token_t token_t;
struct token_t {
  token_kind_t kind;
  token_t* prev;
  token_t* next;

  source_t* src;
  char* str;
  size_t len;
  size_t pos;

  mt_object* value;
};

token_t* token_new(token_kind_t kind, token_t* prev, char* str,
                   size_t len, size_t pos);
