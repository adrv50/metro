#pragma once

#include "types.h"
#include "source.h"
#include "object.h"

typedef u8 token_kind_t;

enum {
  TOK_INT,
  TOK_FLOAT,
  TOK_CHAR,
  TOK_STRING,
  TOK_KEYWORD,
  TOK_IDENTIFIER,
  TOK_PUNCTUATER,
  TOK_END
};

typedef struct token_tag {
  token_kind_t kind;
  struct token_tag* prev;
  struct token_tag* next;

  source_t* src;
  char* str;
  size_t len;
  size_t pos;

  mt_object* value;
} token_t;

token_t* token_new(token_kind_t kind, token_t* prev, char* str, size_t len,
                   size_t pos);
