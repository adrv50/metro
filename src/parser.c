#include <assert.h>
#include <string.h>

#include "metro.h"

typedef struct {
  source_t* src;
  token_t* list;
  token_t* endtok;
  token_t* cur;
} parser_ctx;

parser_ctx parser_new(source_t* src, token_t* list) {
  parser_ctx ctx = {};

  ctx.src = src;
  ctx.list = list;
  ctx.endtok = list;
  ctx.cur = list;

  while (ctx.endtok && ctx.endtok->next)
    ctx.endtok = ctx.endtok->next;

  return ctx;
}

static parser_ctx ctx;

static token_t* getcur() {
  return ctx.cur;
}

static bool check() {
  return getcur()->kind != TOK_END;
}

static void next() {
  ctx.cur = ctx.cur->next;
}

static bool match(char const* str) {
  size_t len = strlen(str);

  return getcur()->len >= len && strncmp(str, getcur()->str, len) == 0;
}

static bool eat(char const* str) {
  if (match(str)) {
    next();
    return true;
  }

  return false;
}

static void expect_keep(char const* str) {
  if (!match(str)) {
    mt_abort_with(mt_new_error_from_token(ERR_UNEXPECTED_TOKEN,
                                          "unexpected token", getcur()));
  }
}

static void expect(char const* str) {
  expect_keep(str);
  next();
}

static mt_node_t* p_stmt();
static mt_node_t* expect_block() {
  expect_keep("{");
  return p_stmt();
}

static token_t* expect_identifier() {
  token_t* tok = getcur();

  if (tok->kind != TOK_IDENTIFIER) {
    mt_abort_with(mt_new_error_from_token(ERR_UNEXPECTED_TOKEN,
                                          "expected identifier", tok));
  }

  next();
  return tok;
}

static mt_node_t* expect_typename() {
  mt_node_t* node = node_new(ND_TYPENAME);

  node->tok = expect_identifier();

  node->name = node->tok->str;
  node->len = node->tok->len;

  node->type_is_const = eat("const");

  return node;
}

static mt_node_t* p_factor() {
  token_t* tok = getcur();
  mt_node_t* node;

  next();

  switch (tok->kind) {
  case TOK_INT:
  case TOK_FLOAT:
  case TOK_CHAR:
  case TOK_STRING:
    node = node_new_with_token(ND_VALUE, tok);
    node->value = tok->value;
    break;

  case TOK_IDENTIFIER:
    node = node_new_with_token(ND_VARIABLE, tok);

    if (eat("(")) {
      todo_impl;
    }

    break;

  default:
    mt_abort_with(
        mt_new_error_from_token(ERR_INVALID_SYNTAX, "invalid syntax", tok));
  }

  return node;
}

static mt_node_t* p_mul() {
  mt_node_t* x = p_factor();
  token_t* tok;

  while (check()) {
    tok = getcur();

    if (eat("*"))
      x = node_new_with_lr(ND_MUL, tok, x, p_factor());
    else if (eat("/"))
      x = node_new_with_lr(ND_DIV, tok, x, p_factor());
    else
      break;
  }

  return x;
}

static mt_node_t* p_add() {
  mt_node_t* x = p_mul();
  token_t* tok;

  while (check()) {
    tok = getcur();

    if (eat("+"))
      x = node_new_with_lr(ND_ADD, tok, x, p_mul());
    else if (eat("-"))
      x = node_new_with_lr(ND_SUB, tok, x, p_mul());
    else
      break;
  }

  return x;
}

static mt_node_t* p_expr() {
  return p_add();
}

static mt_node_t* p_stmt() {
  mt_node_t* node = NULL;
  bool closed = false;

  token_t* token = getcur();

  //
  // block
  //
  if (eat("{")) {
    node = node_new(ND_BLOCK);

    if (eat("}"))
      return node;

    while (check() && !(closed = eat("}"))) {
      node_append(node, p_stmt());
    }

    if (!closed)
      mt_abort_with(mt_new_error_from_token(ERR_NOT_CLOSED_BRACKETS,
                                            "not closed", token));
  }

  //
  //  let: variable declaration
  //
  //  syntax:
  //    let a : type = value;
  //
  //  layout:
  //    name, len     --->  a
  //    child[0]      --->  type
  //    child[1]      --->  value   or null
  //
  else if (eat("let")) {
  }

  //
  //  if-statement
  //
  else if (eat("if")) {
    node = node_new(ND_IF);

    // cond
    node_append(node, p_expr());

    // true-block
    expect_keep("{");
    node_append(node, p_stmt());

    // "else if" or "else {"
    if (eat("else")) {
      if (!match("if"))
        expect_keep("{");

      node_append(node, p_stmt());
    }
    else
      node_append(node, NULL);
  }

  //
  // expr
  //
  else {
    node = p_expr();
    expect(";");
  }

  return node;
}

static mt_node_t* p_top() {
  token_t* tok = getcur();

  //
  //  enum    :=  "enum" <name: ident> "{"
  //              <name: ident> ("(" <type: typename> ")")? ","
  //              "}"
  //
  if (eat("enum")) {
    todo_impl;
  }

  //
  //  struct  :=  "struct" <name: ident> "{"
  //              (<name: ident> ":" <type: typename>)*
  //              "}"
  //
  else if (eat("struct")) {
    todo_impl;
  }

  //
  //  fn: function definition
  //
  //  syntax:
  //    "fn" <name: ident>
  //      "(" params... ")" ("->" <rettype: type>)? <block>
  //
  //  layout:
  //    name, len     = name
  //    child[0]      = return type
  //    child[1]      = block
  //    child[2 <=]   = params
  //
  else if (eat("fn")) {
    mt_node_t* func = node_new_with_token(ND_FUNCTION, tok);

    tok = expect_identifier();

    func->name = tok->str;
    func->len = tok->len;

    mt_node_t** rettype = node_append(func, NULL);
    mt_node_t** block = node_append(func, NULL);

    expect("(");

    if (!eat(")")) {
      do {
        mt_node_t* param = *node_append(
            func, node_new_with_token(ND_PARAM, expect_identifier()));

        expect(":");

        node_append(param, expect_typename());
      } while (eat(","));

      expect(")");
    }

    if (eat("->")) {
      *rettype = expect_typename();
    }

    *block = expect_block();

    assert(func->child->count >= 2);
    assert(*block != NULL);

    return func;
  }

  //
  // statement or expression
  //
  return p_stmt();
}

mt_node_t* parser_parse(source_t* src, token_t* toklist) {
  ctx = parser_new(src, toklist);

  mt_node_t* nd = node_new(ND_PROGRAM);

  while (check()) {
    node_append(nd, p_top());
  }

  return nd;
}