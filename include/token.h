#pragma once

#include "types.h"
#include "object.h"
#include "source.h"

typedef u8 mt_token_kind;

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

typedef struct mt_token mt_token;

struct mt_token {
  mt_token_kind kind;
  struct mt_token* prev;
  struct mt_token* next;

  source_file* src;
  char* str;
  size_t len;
  size_t pos;

  mt_object* value;
};

mt_token* token_new(mt_token_kind kind, mt_token* prev, char* str,
                    size_t len, size_t pos);
