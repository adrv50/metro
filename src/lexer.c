#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <ctype.h>
#include <errno.h>

#include "alert.h"
#include "lexer.h"
#include "metro.h"

static char* punctuaters[] = {
  "<<=",
  ">>=",
  "<<",
  ">>",
  "<=",
  ">=",
  "==",
  "!=",
  "..",
  "&&",
  "||",
  "->",
  "<",
  ">",
  "+",
  "-",
  "/",
  "*",
  "%",
  "=",
  ";",
  ":",
  ",",
  ".",
  "[",
  "]",
  "(",
  ")",
  "{",
  "}",
  "!",
  "?",
  "&",
  "^",
  "|",
};

static size_t const punctuaters_size =
  sizeof(punctuaters) / sizeof(char const*);

token_t* token_new(token_kind_t kind, token_t* prev, char* str, size_t len, size_t pos) {
  token_t* tok = calloc(1, sizeof(token_t));

  tok->kind   = kind;
  tok->prev   = prev;
  tok->next   = NULL;
  tok->str    = str;
  tok->len    = len;
  tok->pos    = pos;

  if( prev )
    prev->next = tok;

  return tok;
}

source_t* source_new(char* path) {
  FILE* fp = fopen(path, "r");

  if( fp == NULL ) {
    int err = errno;

    switch( err ) { 
      case ENOENT:
        fprintf(stderr, "cannot open '%s'", path);
        break;
    }

    return NULL;
  }

  source_t* s = calloc(1, sizeof(source_t));
  s->path = path;

  fseek(fp, 0, SEEK_END);
  s->length = s->size = ftell(fp);

  s->data = malloc(s->size + 2);

  fseek(fp, 0, SEEK_SET);

  if( fread(s->data, s->size, 1, fp) != 1 )
    abort();

  if( s->size == 0 || s->data[s->size - 1] != '\n' )
    s->data[s->size++] = '\n';

  s->data[s->size] = 0;

  fclose(fp);

  return s;
}

void source_free(source_t* s) {
  free(s->data);
  free(s);
}

mtlexer* lexer_new(source_t* src) {
  mtlexer* lexer = calloc(1, sizeof(mtlexer));

  lexer->src        = src;
  lexer->position   = 0;
  lexer->length     = lexer->src->length;

  // alertfmt("%s\n", lexer->src->data);

  return lexer;
}

void lexer_free(mtlexer* lx) {
  source_free(lx->src);
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
  while( lx_check(lx) && isspace(lx_peek(lx)) )
    lx->position++;
}

static bool lx_match(mtlexer* lx, char* str, size_t len) {
  return lx->position + len <= lx->length
    && strncmp(lx_cur_ptr(lx), str, len) == 0;
}

static bool lx_eatstr(mtlexer* lx, char* str, size_t len) {
  if( lx_match(lx, str, len) ) {
    lx->position += len;
    return true;
  }

  return false;
}

static bool lx_eat(mtlexer* lx, char c) {
  if( lx_peek(lx) == c ) {
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

  while( lx_check(lx) ) {
    char c = lx_peek(lx);

    for( size_t i = 0; i < pattern_count; i++ ) {
      if( ranges[i * 2] <= c && c <= ranges[i * 2 + 1] )
        goto _label_continue;
    }

    break;

  _label_continue:
    lx->position++;
    passed++;
  }

  return passed;
}

static token_t* lx_eat_punctuater(mtlexer* lx, token_t* prev) {
  for( size_t i = 0; i < punctuaters_size; i++ ) {
    size_t len = strlen(punctuaters[i]);

    if( lx_match(lx, punctuaters[i], len) ) {
      token_t* tok = token_new(TOK_PUNCTUATER, prev, punctuaters[i], len, lx->position);

      lx->position += len;

      return tok;
    }
  }

  return NULL;
}

token_t* lexer_lex(mtlexer* lx) {
  token_t   top = { };
  token_t*  cur = &top;

  lx_pass_space(lx);

  while( lx_check(lx) ) {
    char    c     = lx_peek(lx);
    char*   str   = lx_cur_ptr(lx);
    size_t  pos   = lx->position;

    // comment line
    if( lx_eatstr(lx, "//", 2) ) {
      while( lx_check(lx) && lx_peek(lx) != '\n' )
        lx->position++;
    }

    // comment block
    else if( lx_eatstr(lx, "/*", 2) ) {
      while( lx_check(lx) && !lx_eatstr(lx, "*/", 2) )
        lx->position++;
    }

    // int or float
    else if( isdigit(c) ) {
      cur = token_new(TOK_INT, cur, str, lx_pass(lx, "09"), pos);

      // float
      if( lx_eat(lx, '.') ) {
        cur->kind = TOK_FLOAT;
        cur->len += lx_pass(lx, "09") + lx_eat(lx, 'f') + 1;

        cur->val_f = atof(str);
      }
      // int
      else {
        cur->val = atoi(str);
      }
    }

    // char
    else if( lx_eat(lx, '\'') ) {
      bool closed;

      while( !(closed = lx_peek(lx) == '\'') )
        lx->position++;

      if( !closed ) {
        mt_abort_with(mt_new_error(ERR_INVALID_TOKEN, "not closed literal", pos));
      }
    }

    // string
    else if( lx_eat(lx, '"') ) {
      bool closed;

      while( !(closed = lx_eat(lx, '"')) )
        lx->position++;

      if( !closed ) {
        mt_abort_with(mt_new_error(ERR_INVALID_TOKEN, "not closed literal", pos));
      }
    }

    // identifier
    else if( c == '_' || isalpha(c) ) {
      cur = token_new(
        TOK_IDENTIFIER, cur, str, lx_pass(lx, "__09azAZ"), pos);
    }

    // punctuater
    else if( !(cur = lx_eat_punctuater(lx, cur)) ) {
      mt_abort_with(mt_new_error(ERR_INVALID_TOKEN, "invalid token", pos));
    }

    lx_pass_space(lx);
  }

  token_new(TOK_END, cur, NULL, 0, 0);

  // set source pointer
  for( token_t* t = top.next; t && t->next; t = t->next )
    t->src = lx->src;

  return top.next;
}

