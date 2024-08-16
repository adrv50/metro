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

static bool check() {
  return getcur()->kind != TOK_END;
}

static void save() {
  saved = getcur();
}

static void next() {
  ctx.cur = ctx.cur->next;
}

static bool match(char const* str) {
  size_t len = strlen(str);

  return
    getcur()->len >= len && strncmp(str, getcur()->str, len) == 0;
}

static bool eat(char const* str) {
  if( match(str) ) {
    next();
    return true;
  }

  return false;
}

static void expect_keep(char const* str) {
  if( !match(str) ) {
    mt_abort_with(mt_new_error_from_token(
      ERR_UNEXPECTED_TOKEN, "unexpected token", getcur()));
  }
}

static void expect(char const* str) {
  expect_keep(str);
  next();
}

static node_t* p_factor() {
  token_t* tok = getcur();
  node_t* node;

  next();

  switch( tok->kind ) {
    case TOK_INT:
    case TOK_FLOAT:
    case TOK_CHAR:
    case TOK_STRING:
      node = node_new_with_token(ND_VALUE, tok);
      break;

    case TOK_IDENTIFIER:
      node = node_new_with_token(ND_VARIABLE, tok);

      // todo: parse arguments if ate "("

      break;

    default:
      mt_abort_with(mt_new_error_from_token(
        ERR_INVALID_SYNTAX, "invalid syntax", tok));
  }

  return node;
}

static node_t* p_mul() {
  node_t* x = p_factor();
  token_t* tok;

  while( check() ) {
    tok = getcur();

    if( eat("*") )
      x = node_new_with_lr(ND_MUL, tok, x, p_factor());
    else if( eat("/") )
      x = node_new_with_lr(ND_DIV, tok, x, p_factor());
    else
      break;
  }

  return x;
}

static node_t* p_add() {
  node_t* x = p_mul();
  token_t* tok;

  while( check() ) {
    tok = getcur();

    if( eat("+") )
      x = node_new_with_lr(ND_ADD, tok, x, p_mul());
    else if( eat("-") )
      x = node_new_with_lr(ND_SUB, tok, x, p_mul());
    else
      break;
  }

  return x;
}

static node_t* p_expr() {
  return p_add();
}

static node_t* p_stmt() {
  node_t* node = NULL;
  bool closed = false;

  token_t* token = getcur();

  // block
  if( eat("{") ) {
    node = node_new(ND_BLOCK);

    if( eat("}") )
      return node;

    while( check() && !(closed = eat("}")) ) {
      node_append(node, p_stmt());
    }

    if( !closed )
      mt_abort_with(mt_new_error_from_token(ERR_NOT_CLOSED_BRACKETS, "not closed", token));
  }

  // if
  else if( eat("if") ) {
    node = node_new(ND_IF);

    // cond
    node_append(node, p_expr());

    // true-block
    expect_keep("{");
    node_append(node, p_stmt());

    // "else if" or "else {"
    if( eat("else") ) {
      if( !match("if") )
        expect_keep("{");

      node_append(node, p_stmt());
    }
    else
      node_append(node, NULL);
  }

  // expr
  else {
    node = p_expr();
    expect(";");
  }

  return node;
}

node_t*  parser_parse(source_t* src, token_t* toklist) {
  ctx = parser_new(src, toklist);

  node_t* nd = node_new(ND_PROGRAM);

  while( check() ) {
    node_append(nd, p_stmt());
  }

  return nd;
}