#include <string.h>
#include "parser.h"
#include "lexer.h"
#include "metro.h"

typedef struct parser_ctx parser_ctx;
struct parser_ctx {
  source_t*   src;
  token_t*    list;
  token_t*    endtok;
  token_t*    cur;
  
};

parser_ctx parser_new(source_t* src, token_t* list) {
  parser_ctx ctx = { };

  ctx.src     = src;
  ctx.list    = list;
  ctx.endtok  = list;
  ctx.cur     = list;

  while( ctx.endtok && ctx.endtok->next )
    ctx.endtok = ctx.endtok->next;

  return ctx;
}

static parser_ctx     ctx;
static token_t*       saved;

static token_t* getcur() {
  return ctx.cur;
}

static void save() {
  saved = getcur();
}

static void next() {
  ctx.cur = ctx.cur->next;
}

static bool eat(char const* str) {
  int len = strlen(str);

  if( getcur()->len >= len && strncmp(str, getcur()->len, len) == 0 ) {
    next();
    return true;
  }

  return false;
}

static void expect(char const* str) {
  if( !eat(str) ) {
    
  }
}

static node_t* p_factor() {
  
}

static node_t* p_mul() {

}

static node_t* p_add() {

}

static node_t* p_expr() {

}

node_t*  parser_parse(source_t* src, token_t* toklist) {
  ctx = parser_new(src, toklist);

  return p_expr();
}