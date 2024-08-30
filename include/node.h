#pragma once

#include "token.h"
#include "vector.h"
#include "object.h"

#define nd_get_child(_ND, _INDEX)                                    \
  (vector_get_as(mt_node*, (_ND)->child, _INDEX))

#define _N0(N) nd_get_child(N, 0)
#define _N1(N) nd_get_child(N, 1)
#define _N2(N) nd_get_child(N, 2)

#define nd_lhs(n) _N0(n)
#define nd_rhs(n) _N1(n)

#define nd_callfunc_callee(n) _N0(n)

#define nd_let_type(n) _N0(n)
#define nd_let_init(n) _N1(n)

#define nd_if_cond(n) _N0(n)
#define nd_if_true(n) _N1(n)
#define nd_if_false(n) _N2(n)

#define nd_func_rettype(n) _N0(n)
#define nd_func_block(n) _N1(n)
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

  ND_INDEXREF,      // [ ]
  ND_MEMBER_ACCESS, // .

  ND_NOT, // !

  ND_MUL, // *
  ND_DIV, // /
  ND_MOD, // %

  ND_ADD, // +
  ND_SUB, // -

  ND_LSHIFT, // <<
  ND_RSHIFT, // >>

  ND_EQUAL,        // ==
  ND_BIGGER,       // >
  ND_BIGGER_OR_EQ, // >=

  ND_BIT_AND, // &
  ND_BIT_XOR, // ^
  ND_BIT_OR,  // |

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
  mt_token* tok;

  vector* child; // vector<mt_node>

  char* name;
  size_t len;

  union {
    // when ND_VALUE
    mt_object* value;

    // ND_VARIABLE (or expr)
    size_t index;

    // when self is ND_TYPENAME
    bool type_is_const;
  };

} mt_node;

mt_node* node_new(mt_node_kind kind);
mt_node* node_new_with_token(mt_node_kind k, mt_token* tok);
mt_node* node_new_with_lr(mt_node_kind k, mt_token* tok, mt_node* lhs,
                          mt_node* rhs);

void node_free(mt_node* node);

mt_node** node_append(mt_node* node, mt_node* item);

bool node_is_same_name(mt_node* node, char const* name);

void print_node(mt_node* nd);
