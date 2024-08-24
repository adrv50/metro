#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "alert.h"
#include "eval.h"
#include "vector.h"

static bool check_valid_operator(mt_node_kind kind, mt_type_info_t left,
                                 mt_type_info_t right) {

                                  

}

static mt_object* evaluate(mt_node_t* node) {
  static int* case_labels[] = {
      [ND_VALUE] = &&case_value,
      [ND_PROGRAM] = &&case_program,
  };

  static int* op_expr_labels[] = {
      [ND_MUL] = &&case_mul,
      [ND_ADD] = &&case_add,
  };

  mt_object* result = NULL;

  // alertfmt("%hhd\n", node->kind);

  if (node->kind >= _NDKIND_BEGIN_OF_LR_OP_EXPR_ &&
      node->kind <= _NDKIND_END_OF_LR_OP_EXPR_)
    goto case_lr_operator_expr;

  goto* case_labels[node->kind];

case_value:
  return node->value;

case_program:
  for (size_t i = 0; i < node->child->count; i++)
    result = evaluate(nd_get_child(node, i));

  return result;

case_lr_operator_expr:

  mt_object* left = evaluate(nd_lhs(node));
  mt_object* right = evaluate(nd_rhs(node));

  goto* op_expr_labels[node->kind];

case_mul:

case_add:
  left->vi += right->vi;

  return left;
}

void mt_eval_init(void) {
}

void mt_eval_exit(void) {
}

mt_object* mt_eval_evalfull(mt_node_t* node) {
  return evaluate(node);
}
