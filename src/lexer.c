#include <assert.h>
#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>

#include "alert.h"
#include "lexer.h"
#include "metro.h"
#include "utf-convert.h"

#include "object.h"

static char* punctuaters[] = {
    "<<=", ">>=", "<<", ">>", "<=", ">=", "==", "!=", "..",
    "&&",  "||",  "->", "<",  ">",  "+",  "-",  "/",  "*",
    "%",   "=",   ";",  ":",  ",",  ".",  "[",  "]",  "(",
    ")",   "{",   "}",  "!",  "?",  "&",  "^",  "|",
};

static size_t const punctuaters_size =
    sizeof(punctuaters) / sizeof(char const*);

mt_token* token_new(mt_token_kind kind, mt_token* prev, char* str,
                    size_t len, size_t pos) {
  mt_token* tok = calloc(1, sizeof(mt_token));

  tok->kind = kind;
  tok->prev = prev;
  tok->next = NULL;
  tok->str = str;
  tok->len = len;
  tok->pos = pos;

  if (prev)
    prev->next = tok;

  return tok;
}

mtlexer* lexer_new(source_file* src) {
  mtlexer* lexer = calloc(1, sizeof(mtlexer));

  lexer->src = src;
  lexer->position = 0;
  lexer->length = lexer->src->length;

  // alertfmt("%s\n", lexer->src->data);

  return lexer;
}

void lexer_free(mtlexer* lx) {
  source_file_free(lx->src);
  free(lx);
}

static bool lx_check(mtlexer* lx) {
  return lx->position < lx->length;
}

static char lx_peek(mtlexer* lx) {
  return lx_check(lx) ? lx->src->data[lx->position] : 0;
}

static char* lx_cur_ptr(mtlexer* lx) {
  return lx->src->data + lx->position;
}

static void lx_pass_space(mtlexer* lx) {
  while (lx_check(lx) && isspace(lx_peek(lx)))
    lx->position++;
}

static bool lx_match(mtlexer* lx, char* str, size_t len) {
  return lx->position + len <= lx->length &&
         strncmp(lx_cur_ptr(lx), str, len) == 0;
}

static bool lx_eatstr(mtlexer* lx, char* str, size_t len) {
  if (lx_match(lx, str, len)) {
    lx->position += len;
    return true;
  }

  return false;
}

static bool lx_eat(mtlexer* lx, char c) {
  if (lx_peek(lx) == c) {
    lx->position++;
    return true;
  }

  return false;
}

//
// skip all characters within any ranges
//
static size_t lx_pass(mtlexer* lx, char* ranges) {
  size_t pattern_count = strlen(ranges);
  size_t passed = 0;

  assert(pattern_count >= 2);
  assert(pattern_count % 2 == 0);

  pattern_count /= 2;

  while (lx_check(lx)) {
    char c = lx_peek(lx);

    for (size_t i = 0; i < pattern_count; i++) {
      if (ranges[i * 2] <= c && c <= ranges[i * 2 + 1])
        goto _label_continue;
    }

    break;

  _label_continue:
    lx->position++;
    passed++;
  }

  return passed;
}

static mt_token* lx_eat_punctuater(mtlexer* lx, mt_token* prev) {
  for (size_t i = 0; i < punctuaters_size; i++) {
    size_t len = strlen(punctuaters[i]);

    if (lx_match(lx, punctuaters[i], len)) {
      mt_token* tok = token_new(TOK_PUNCTUATER, prev, punctuaters[i],
                                len, lx->position);

      lx->position += len;

      return tok;
    }
  }

  return NULL;
}

mt_token* lexer_lex(mtlexer* lx) {
  mt_token top = {};
  mt_token* cur = &top;

  lx_pass_space(lx);

  while (lx_check(lx)) {
    char c = lx_peek(lx);
    char* str = lx_cur_ptr(lx);
    size_t pos = lx->position;

    // comment line
    if (lx_eatstr(lx, "//", 2)) {
      while (lx_check(lx) && lx_peek(lx) != '\n')
        lx->position++;
    }

    // comment block
    else if (lx_eatstr(lx, "/*", 2)) {
      while (lx_check(lx) && !lx_eatstr(lx, "*/", 2))
        lx->position++;
    }

    // hexadecimal
    else if (lx_eatstr(lx, "0x", 2) || lx_eatstr(lx, "0X", 2)) {
      cur = token_new(TOK_INT, cur, str, lx_pass(lx, "09afAF"), pos);
      cur->value = mt_obj_new_int(strtoll(str, NULL, 16));
    }

    // int or float
    else if (isdigit(c)) {
      cur = token_new(TOK_INT, cur, str, lx_pass(lx, "09"), pos);

      // float
      if (lx_eat(lx, '.')) {
        cur->kind = TOK_FLOAT;
        cur->len += lx_pass(lx, "09") + lx_eat(lx, 'f') + 1;

        cur->value = mt_obj_new_float(strtod(str, NULL));
      }
      // int
      else {
        cur->value = mt_obj_new_int(strtoll(str, NULL, 10));
      }
    }

    // char
    else if (lx_eat(lx, '\'')) {
      bool closed = false;

      while (!(closed = lx_peek(lx) == '\''))
        lx->position++;

      if (!closed) {
        mt_abort_with(mt_new_error(ERR_INVALID_TOKEN,
                                   "not closed literal", pos));
      }

      cur = token_new(TOK_CHAR, cur, str, lx->position - pos, pos);

      todo_impl;
      cur->value = mt_obj_new_char(0);
    }

    // string
    else if (lx_eat(lx, '"')) {
      bool closed = false;

      while (lx_check(lx) && !(closed = lx_eat(lx, '"')))
        lx->position++;

      if (!closed) {
        mt_abort_with(mt_new_error(ERR_INVALID_TOKEN,
                                   "not closed literal", pos));
      }

      cur = token_new(TOK_STRING, cur, str + 1,
                      lx->position - pos - 2, pos);

      // make null-terminated string by cur->str
      char buf[cur->len + 1];

      memcpy(buf, cur->str, cur->len);
      buf[cur->len] = 0;

      // get correctly length of cur->str
      size_t const length = get_count_of_utf8(buf);

      cur->value = mt_obj_new(mt_type_info_new(TYPE_STRING));
      cur->value->vs = vector_new_with_count(sizeof(u16), length);

      utf8_to_utf16(cur->value->vs->_data, (u8*)buf, length);

      // alert;
      // alertfmt("%zu", length);
      // alertfmt("cur->str = \"%.*s\"", (int)cur->len, cur->str);

      // for (size_t i = 0; i <= length; i++) {
      //   alertfmt("%04X", vector_get_as(u16, cur->value->vs, i));
      // }
    }

    // identifier
    else if (c == '_' || isalpha(c)) {
      cur = token_new(TOK_IDENTIFIER, cur, str,
                      lx_pass(lx, "__09azAZ"), pos);
    }

    // punctuater
    else if (!(cur = lx_eat_punctuater(lx, cur))) {
      mt_abort_with(
          mt_new_error(ERR_INVALID_TOKEN, "invalid token", pos));
    }

    lx_pass_space(lx);
  }

  token_new(TOK_END, cur, NULL, 0, 0);

  // set source pointer
  for (mt_token* t = top.next; t && t->next; t = t->next)
    t->src = lx->src;

  return top.next;
}