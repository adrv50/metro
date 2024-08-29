#include <stdio.h>
#include <string.h>
#include <assert.h>

#include "alert.h"
#include "check.h"
#include "metro.h"

/*

#define MAKE_PTTR(OP, L, R, RS)                                      \
  { ND_##OP, {TYPE_##L}, {TYPE_##R}, {TYPE_##RS}, }

typedef struct {
  mt_node_kind op;
  mt_type_info left;
  mt_type_info right;
  mt_type_info result;
} expr_type_pattern;

// clang-format off

// 足し算、掛け算の場合は左右逆でも一致

static expr_type_pattern expr_pattern_list[] = {
  // str * int = str
  MAKE_PTTR(ADD, STRING, INT, STRING),
};

// clang-format on

*/

mt_type_info type_eval(mt_node* node) {
  if (!node)
    return mt_type_info_new(TYPE_NONE);

  if (node->kind > _NDKIND_BEGIN_OF_LR_OP_EXPR_ &&
      node->kind < _NDKIND_END_OF_LR_OP_EXPR_) {

    mt_type_info lhs = type_eval(nd_lhs(node));
    mt_type_info rhs = type_eval(nd_rhs(node));
    mt_token* tok = node->tok;

    mt_type_kind lk = lhs.kind, rk = rhs.kind;

    bool is_same = lhs.kind == rhs.kind;

    switch (node->kind) {

      //
      // mul
      //
    case ND_MUL:
      if (is_same) {
        switch (lhs.kind) {
        case TYPE_INT:
        case TYPE_FLOAT:
          return lhs;
        }
      }

      // float * int
      if ((lk == TYPE_FLOAT && rk == TYPE_INT) ||
          (lk == TYPE_INT && rk == TYPE_FLOAT))
        return (mt_type_info){TYPE_FLOAT};

      // string * int
      if ((lk == TYPE_INT && rk == TYPE_STRING) ||
          (lk == TYPE_STRING && rk == TYPE_INT))
        return (mt_type_info){TYPE_STRING};

      break;
    }

    mt_abort_with(mt_new_error_from_token(
        ERR_TYPE_MISMATCH, "type mismatch", node->tok));
  }

  switch (node->kind) {
  case ND_VALUE:
    return node->value->typeinfo;

  default:
    todo_impl;
  }

  return mt_type_info_new(TYPE_NONE);
}

void check(mt_node* node) {
}