#include <assert.h>
#include <string.h>

#include "metro.h"
#include "parser.h"

parser_ctx parser_new(source_file* src, mt_token* list) {
  parser_ctx ctx = {};

  ctx.src = src;
  ctx.list = list;
  ctx.endtok = list;
  ctx.cur = list;

  while (ctx.endtok && ctx.endtok->next)
    ctx.endtok = ctx.endtok->next;

  return ctx;
}

static mt_node* new_int_node(i64 val) {
  mt_node* nd = node_new(ND_VALUE);

  nd->value = mt_obj_new_int(val);

  return nd;
}

static mt_node* new_zero() {
  return new_int_node(0);
}

static parser_ctx ctx;

static mt_token* getcur() {
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

  return getcur()->len == len &&
         strncmp(str, getcur()->str, len) == 0;
}

static bool eat(char const* str) {
  if (match(str)) {
    ctx.ate = getcur();
    next();
    return true;
  }

  return false;
}

static void expect_keep(char const* str) {
  if (!match(str)) {
    mt_abort_with(mt_new_error_from_token(
        ERR_UNEXPECTED_TOKEN, "unexpected token", getcur()));
  }
}

static void expect(char const* str) {
  expect_keep(str);
  next();
}

static mt_node* p_stmt();
static mt_node* expect_block() {
  expect_keep("{");
  return p_stmt();
}

static mt_token* expect_identifier() {
  mt_token* tok = getcur();

  if (tok->kind != TOK_IDENTIFIER) {
    mt_abort_with(mt_new_error_from_token(
        ERR_UNEXPECTED_TOKEN, "expected identifier", tok));
  }

  next();
  return tok;
}

static mt_node* p_scope_resol();
static mt_node* expect_identifier_nd() {
  mt_node* nd = p_scope_resol();

  if (nd->kind != ND_IDENTIFIER && nd->kind != ND_SCOPE_RESOLUTION) {
    mt_abort_with(mt_new_error_from_token(
        ERR_UNEXPECTED_TOKEN, "expected identifier", nd->tok));
  }

  return nd;
}

//
// parse typename
//
static void parse_typename(typename_node_data_t* data) {
  data->tok = expect_identifier();
  data->name = data->tok->str;
  data->len = data->tok->len;

  data->scope_resol = vector_new(sizeof(typename_node_data_t));
  data->params = vector_new(sizeof(typename_node_data_t));

  while (eat("::")) {
    parse_typename(vector_append(
        data->scope_resol, calloc(1, sizeof(typename_node_data_t))));
  }

  if (eat("<")) {
    do {
      parse_typename(vector_append(
          data->params, calloc(1, sizeof(typename_node_data_t))));
    } while (check() && eat(","));

    if (match(">>")) {
      mt_token* tok = getcur();

      mt_token* temp =
          token_new(TOK_PUNCTUATER, tok, ">", 1, tok->pos + 1);

      temp->next = tok->next;

      tok->len = 1;
      tok->next = temp;
    }

    expect(">");
  }

  data->is_const = eat("const");
}

static mt_node* expect_typename() {
  mt_node* node = node_new(ND_TYPENAME);

  node->typend = calloc(1, sizeof(typename_node_data_t));

  parse_typename(node->typend);

  node->tok = node->typend->tok;
  node->name = node->typend->name;
  node->len = node->typend->len;

  return node;
}

static mt_node* p_expr();

// ------------------------------
//  p_factor
// ------------------------------
static mt_node* p_factor() {
  mt_token* tok = getcur();

  switch (tok->kind) {
  case TOK_INT:
  case TOK_FLOAT:
  case TOK_CHAR:
  case TOK_STRING: {
    mt_node* node = node_new_with_token(ND_VALUE, tok);

    node->value = tok->value;

    next();

    return node;
  }
  }

  mt_abort_with(mt_new_error_from_token(ERR_INVALID_SYNTAX,
                                        "invalid syntax", tok));
}

// ------------------------------
//  p_ident
//
//  identifier   :=  ident ("<" scope_resol ("," scope_rosol)* ">")?
// ------------------------------
static mt_node* p_scope_resol();
static mt_node* p_ident() {
  mt_token* tok = getcur();

  if (tok->kind == TOK_IDENTIFIER) {
    mt_node* node = node_new_with_token(ND_IDENTIFIER, tok);

    node->name = node->tok->str;
    node->len = node->tok->len;

    next();

    if (eat("<")) {
      do {
        node_append(node, p_scope_resol());
      } while (eat(","));

      tok = getcur();

      if (match(">>")) {
        mt_token* temp =
            token_new(TOK_PUNCTUATER, tok, ">", 1, tok->pos + 1);

        temp->next = tok->next;

        tok->len = 1;
        tok->next = temp;
      }

      expect(">");
    }

    return node;
  }

  return p_factor();
}

// ------------------------------
//  p_scope_resol
//
//  scope_resol  := identifier ("::" identifier)*
// ------------------------------
static mt_node* p_scope_resol() {
  mt_node* x = p_ident();

  while (check() && eat("::")) {
    mt_token* tok = ctx.ate;

    x = node_new_with_lr(ND_SCOPE_RESOLUTION, tok, x, p_ident());
  }

  return x;
}

// ------------------------------
//  p_indexref
//
//  operator:
//    1:  [ ]     index reference
//    2:  .       member access
//    3:  ( )     call function
// ------------------------------
static mt_node* p_indexref() {
  mt_node* x = p_scope_resol();

  while (check()) {
    mt_token* tok = getcur();

    if (eat(".")) {
      x = node_new_with_lr(ND_MEMBER_ACCESS, tok, x, p_scope_resol());
    }
    else if (eat("[")) {
      x = node_new_with_lr(ND_INDEXREF, tok, x, p_scope_resol());
      expect("]");
    }
    else if (eat("(")) {
      x = node_new_with_lr(ND_CALLFUNC, tok, x, NULL);

      if (!eat(")")) {
        do {
          node_append(x, p_expr());
        } while (check() && eat(","));

        expect(")");
      }
    }
    else
      break;
  }

  return x;
}

// ------------------------------
//  p_unary
//
//  operator:
//    1:  !
//    2:  -
//    2:  +
// ------------------------------
static mt_node* p_unary() {
  mt_token* tok = getcur();

  if (eat("!"))
    return node_new_with_lr(ND_NOT, tok, p_indexref(), NULL);

  if (eat("-"))
    return node_new_with_lr(ND_SUB, tok, new_zero(), p_indexref());

  eat("+"); // optional

  return p_indexref();
}

// ------------------------------
//  p_mul
//
//  operator:
//    1.  *
//    2.  /
//    3.  %
// ------------------------------
static mt_node* p_mul() {
  mt_node* x = p_unary();

  while (check()) {
    mt_token* tok = getcur();

    if (eat("*"))
      x = node_new_with_lr(ND_MUL, tok, x, p_unary());
    else if (eat("/"))
      x = node_new_with_lr(ND_DIV, tok, x, p_unary());
    else if (eat("%"))
      x = node_new_with_lr(ND_MOD, tok, x, p_unary());
    else
      break;
  }

  return x;
}

// ------------------------------
//  p_add
//
//  operator:
//    1.  +
//    2.  -
// ------------------------------
static mt_node* p_add() {
  mt_node* x = p_mul();
  mt_token* tok;

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

// ------------------------------
//  p_shift
//
//  operator:
//    1.  <<
//    2.  >>
// ------------------------------
static mt_node* p_shift() {
  mt_node* x = p_add();

  while (check()) {
    mt_token* tok = getcur();

    if (eat("<<"))
      x = node_new_with_lr(ND_LSHIFT, tok, x, p_add());
    else if (eat(">>"))
      x = node_new_with_lr(ND_RSHIFT, tok, x, p_add());
    else
      break;
  }

  return x;
}

// ------------------------------
//  p_compare
//
//  operator:
//    1.  ==
//    2.  !=
//    3.  >
//    4.  <
//    5.  >=
//    6.  <=
// ------------------------------
static mt_node* p_compare() {
  mt_node* x = p_shift();

  while (check()) {
    mt_token* tok = getcur();

    // a == b
    if (eat("=="))
      x = node_new_with_lr(ND_EQUAL, tok, x, p_shift());

    // a != b
    else if (eat("!="))
      // --> !(a == b)
      x = node_new_with_lr(
          ND_NOT, tok, node_new_with_lr(ND_EQUAL, tok, x, p_shift()),
          NULL);

    // a > b
    else if (eat(">"))
      x = node_new_with_lr(ND_BIGGER, tok, x, p_shift());

    // a < b
    else if (eat("<"))
      // --> b > a
      x = node_new_with_lr(ND_BIGGER, tok, p_shift(), x);

    // a >= b
    else if (eat(">="))
      x = node_new_with_lr(ND_BIGGER_OR_EQ, tok, x, p_shift());

    // a <= b
    else if (eat("<="))
      // --> b >= a
      x = node_new_with_lr(ND_BIGGER_OR_EQ, tok, p_shift(), x);

    else
      break;
  }

  return x;
}

// ------------------------------
//  p_bit_calc
// ------------------------------
static mt_node* p_bit_calc() {
  mt_node* x = p_compare();
  mt_token* tok = getcur();

  if (eat("&"))
    x = node_new_with_lr(ND_BIT_AND, tok, x, p_compare());
  else if (eat("^"))
    x = node_new_with_lr(ND_BIT_XOR, tok, x, p_compare());
  else if (eat("|"))
    x = node_new_with_lr(ND_BIT_OR, tok, x, p_compare());

  return x;
}

// ------------------------------
//  p_assign
// ------------------------------
static mt_node* p_assign() {
  mt_node* x = p_bit_calc();
  mt_token* tok = getcur();

  if (eat("="))
    x = node_new_with_lr(ND_ASSIGN, tok, x, p_assign());

  return x;
}

// ------------------------------
//  p_expr
// ------------------------------
static mt_node* p_expr() {
  return p_assign();
}

// ------------------------------
//  p_stmt
// ------------------------------
static mt_node* p_stmt() {
  mt_node* node = NULL;
  bool closed = false;

  mt_token* token = getcur();

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
  //    name, len     --->  (name)
  //    child[0]      --->  type
  //    child[1]      --->  value   or null
  //
  else if (eat("let")) {
    node = node_new(ND_VARDEF);

    token = expect_identifier();
    node->name = token->str;
    node->len = token->len;

    mt_node** type = node_append(node, NULL);
    mt_node** init = node_append(node, NULL);

    if (eat(":"))
      *type = expect_typename();

    if (eat("="))
      *init = p_expr();

    expect(";");
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

static mt_node* p_top() {
  mt_token* tok = getcur();

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
  //    child[0]    = name  (p_scope_resol)
  //    child[1]    = params
  //    child[2]    = return type
  //    child[3]    = block
  //
  else if (eat("fn")) {
    mt_node* func = node_new_with_token(ND_FUNCTION, tok);

    vector_resize(func->child, 4);

    nd_func_params(func) = node_new(ND_FUNCTION_PARAMS);

    nd_func_name(func) = expect_identifier_nd();

    expect("(");

    if (!eat(")")) {
      do {
        mt_node* param =
            nd_func_add_param(func, node_new(ND_NAME_AND_TYPE));

        param->tok = expect_identifier();

        expect(":");

        node_append(param, expect_typename());

      } while (check() && eat(","));

      expect(")");
    }

    if (eat("->")) {
      nd_func_rettype(func) = expect_typename();
    }

    nd_func_block(func) = expect_block();

    return func;
  }

  //
  // statement or expression
  //
  return p_stmt();
}

mt_node* parser_parse(source_file* src, mt_token* toklist) {
  ctx = parser_new(src, toklist);

  mt_node* nd = node_new(ND_BLOCK);

  while (check()) {
    node_append(nd, p_top());
  }

  return nd;
}