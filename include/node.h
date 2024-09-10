#pragma once

#include "token.h"
#include "vector.h"
#include "object.h"
#include "builtin.h"

// clang-format off

#define nd_get_child(_ND, _INDEX)                                    \
  (vector_get_as(mt_node*, (_ND)->child, _INDEX))

#define _N0(N)    nd_get_child(N, 0)
#define _N1(N)    nd_get_child(N, 1)
#define _N2(N)    nd_get_child(N, 2)
#define _N3(N)    nd_get_child(N, 3)

#define nd_lhs(n)     _N0(n)
#define nd_rhs(n)     _N1(n)

#define nd_callfunc_callee(n) _N0(n)

#define nd_let_type(n)    _N0(n)
#define nd_let_init(n)    _N1(n)

#define nd_if_cond(n)     _N0(n)
#define nd_if_true(n)     _N1(n)
#define nd_if_false(n)    _N2(n)

#define nd_switch_cond(n) _N0(n)
#define nd_switch_(n) _N0(n)

#define nd_func_name(n)           _N0(n)
#define nd_func_params(n)         _N1(n)
#define nd_func_rettype(n)        _N2(n)
#define nd_func_block(n)          _N3(n)
#define nd_func_get_param(n, i)   (nd_get_child(nd_func_params(n), i))
#define nd_func_add_param(n, pn)  (*node_append(nd_func_params(n), pn))

#define nd_callfunc_callee(n)         _N0(n)
#define nd_callfunc_params(n)         _N1(n)
#define nd_callfunc_add_param(n, n2)  (*node_append(nd_callfunc_params(n), n2))
#define nd_callfunc_get_param(n, i)   (nd_get_child(nd_callfunc_params(n), i))
#define nd_callfunc_get_argcount(n)   (nd_callfunc_params(n)->child->count)

// clang-format on

typedef u8 mt_node_kind;

enum {
  ND_TYPENAME,    // type  :=  <name: ident> type_param? "const"?
  ND_TYPE_PARAMS, // type_param  := "<" type ("," type)* ">"

  ND_NAME_AND_TYPE, // ident ":" type

  ND_VALUE,    // value    :=  (token)
  ND_VARIABLE, // ident    :=  TOK_IDENTIFIER

  //
  // identifier   :=  ident type_param?
  ND_IDENTIFIER,

  ND_SCOPE_RESOLUTION,

  ND_MEMBER_ACCESS, // .
  ND_CALLFUNC, // callfunc :=  <ident> "(" <expr> ("," <expr>)* ")"
  ND_CALLFUNC_PARAMS,

_NDKIND_BEGIN_OF_LR_OP_EXPR_,

  ND_INDEXREF,      // [ ]

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

  ND_IF,
  ND_SWITCH,

  ND_RETURN,
  ND_BREAK,
  ND_CONTINUE,

  ND_LOOP,
  ND_FOR,
  ND_WHILE,
  ND_DO_WHILE,

  ND_PARAM, // <name: ident> ":" <type>

  // child = { ret_type, params... }
  ND_FUNCTION,
  ND_FUNCTION_PARAMS,

  ND_ENUM,
  ND_STRUCT,
  ND_CLASS,

  ND_NAMESPACE,

  ND_PROGRAM, // (root)
};

typedef struct {
  mt_token* tok;
  char const* name;
  int len;
  vector* scope_resol; //
  vector* params;      // vector<typename_node_data_t>
  bool is_const;
} typename_node_data_t;

struct mt_ck_checked_log_t;

typedef enum {
  NID_VARIABLE,
  NID_FUNCTION,
  NID_BLT_FUNC,
} nd_identifier_kind;

//
// node_identifier_info
//
typedef struct {
  bool is_valid;
  nd_identifier_kind kind;

  // function
  struct mt_node* callee_nd;
  mt_builtin_func_t const* callee_builtin;

  // variable
  int vdepth;
  int index;
  
} node_identifier_info;

struct __attribute__((__packed__)) mt_node {
  mt_node_kind kind;
  mt_token* tok;

  vector* child; // vector<mt_node*>

  char const* name;
  int len;

  struct mt_ck_checked_log_t* checked;

  union {
    // when ND_IDENTIFIER or ND_SCOPE_RESOLUTION
    node_identifier_info id_info;

    // when ND_VALUE
    mt_object* value;

    // when ND_TYPENAME
    typename_node_data_t* typend;
  };

};

mt_node* node_new(mt_node_kind kind);
mt_node* node_new_with_token(mt_node_kind k, mt_token* tok);
mt_node* node_new_with_lr(mt_node_kind k, mt_token* tok, mt_node* lhs,
                          mt_node* rhs);

void node_free(mt_node* node);

mt_node** node_append(mt_node* node, mt_node* item);

bool node_is_same_name(mt_node* node, char const* name);

void print_node(mt_node* nd);

char* node_to_string(mt_node* node);