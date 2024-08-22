#pragma once

#include "token.h"
#include "vector.h"

#define nd_get_child(_ND, _INDEX)                                              \
  (vector_get_as(mt_node_t*, (_ND)->child, _INDEX))

#define nd_lhs(_ND) nd_get_child(_ND, 0)
#define nd_rhs(_ND) nd_get_child(_ND, 1)

#define nd_if_cond(n) nd_get_child(n, 0)
#define nd_if_true(n) nd_get_child(n, 1)
#define nd_if_false(n) nd_get_child(n, 2)

#define nd_func_rettype(n) nd_get_child(n, 0)
#define nd_func_block(n) nd_get_child(n, 1)
#define nd_func_param_count(n) (n->child->count - 2)
#define nd_func_get_param(n, _i) nd_get_child(n, _i + 2)

typedef u8 mt_node_kind;

enum {
  ND_TYPENAME,    // type  :=  <name: ident> type_param? "const"?
  ND_TYPE_PARAMS, // type_param  := "<" type ("," type)* ">"

  ND_VALUE,    // value    :=  (token)
  ND_VARIABLE, // ident    :=  TOK_IDENTIFIER
  ND_CALLFUNC, // callfunc :=  <ident> "(" <expr> ("," <expr>)* ")"

  _NDKIND_BEGIN_OF_LR_OP_EXPR_,

  ND_MUL, // *
  ND_DIV, // /
  ND_MOD, // %

  ND_ADD, // +
  ND_SUB, // -

  ND_LSHIFT, // <<
  ND_RSHIFT, // >>

  ND_EQUAL,        // ==
  ND_BIGGER,       // <
  ND_BIGGER_OR_EQ, // <=

  _NDKIND_END_OF_LR_OP_EXPR_,

  ND_ASSIGN, // =

  // vardef :=  "let" <name: ident> ":" <type> ("=" <expr>)?
  ND_VARDEF,

  // block  := "{" stmt* "}"
  ND_BLOCK,

  // if     :=  "if" <expr> <block> ("else" (<if> | <block>))?
  ND_IF,

  // for    :=  "for" <ident> "in" <iterable> <block>
  ND_FOR,

  // while  :=  "while" <cond: expr> <block>
  ND_WHILE,

  // child = { type }
  ND_PARAM, // <name: ident> ":" <type>

  // child = { ret_type, params... }
  ND_FUNCTION,

  ND_ENUM,
  ND_STRUCT,

  ND_PROGRAM, // (root)
};

typedef struct __attribute__((__packed__)) {
  mt_node_kind kind;
  token_t* tok;

  vector* child; // vector<mt_node_t*>

  char* name;
  size_t len;

  union {
    // when ND_VALUE
    mt_object* value;

    // ND_VARIABLE
    size_t index;

    // when self is ND_TYPENAME
    bool type_is_const;
  };
} mt_node_t;

mt_node_t* node_new(mt_node_kind kind);
mt_node_t* node_new_with_token(mt_node_kind k, token_t* tok);
mt_node_t* node_new_with_lr(mt_node_kind k, token_t* tok, mt_node_t* lhs,
                            mt_node_t* rhs);
void node_free(mt_node_t* node);

mt_node_t** node_append(mt_node_t* node, mt_node_t* item);

bool node_is_same_name(mt_node_t* node, char const* name);

void print_node(mt_node_t* nd);
