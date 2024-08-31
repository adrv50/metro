#include <stdio.h>
#include <string.h>
#include <assert.h>

#include "alert.h"
#include "check.h"
#include "metro.h"

static inline bool is_contain(mt_type_kind kind, mt_type_kind a,
                              mt_type_kind b) {
  return a == kind || b == kind;
}

static inline bool is_numeric(mt_type_kind k) {
  return k == TYPE_INT || k == TYPE_FLOAT;
}

mt_type_info type_eval(mt_node* node) {
  if (!node)
    return mt_type_info_new(TYPE_NONE);

  if (node->kind > _NDKIND_BEGIN_OF_LR_OP_EXPR_ &&
      node->kind < _NDKIND_END_OF_LR_OP_EXPR_) {

    mt_type_info lhs = type_eval(nd_lhs(node));
    mt_type_info rhs = type_eval(nd_rhs(node));

    mt_type_kind lk = lhs.kind, rk = rhs.kind;

    bool is_same_kind = lhs.kind == rhs.kind;

    if (is_contain(TYPE_NONE, lk, rk))
      goto err;

    switch (node->kind) {
    // add
    case ND_ADD:
      if (is_contain(TYPE_BOOL, lk, rk) ||
          is_contain(TYPE_CHAR, lk, rk) ||
          (is_numeric(lk) != is_numeric(rk)))
        goto err;

    // mul
    case ND_MUL:
      if (is_numeric(lk) && is_numeric(rk)) {
        if (is_contain(TYPE_FLOAT, lk, rk))
          lhs.kind = TYPE_FLOAT;

        break;
      }
      break;
    }

    return lhs;

  err:
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

// ===============================
//    Checker
// ===============================
void check(mt_node* node) {
}