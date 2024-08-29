#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "utils.h"
#include "alert.h"
#include "eval.h"
#include "vector.h"
#include "utf-convert.h"

// static bool check_valid_operator(mt_node_kind kind, mt_type_info_t
// left,
//                                  mt_type_info_t right) {
// }

static inline int is_either_type(mt_type_kind K, mt_object* a,
                                 mt_object* b) {
  if (a->typeinfo.kind == K)
    return 1;

  if (b->typeinfo.kind == K)
    return 2;

  return 0;
}

static inline mt_object* to_float(mt_object* int_obj) {

  switch (int_obj->typeinfo.kind) {
  case TYPE_INT:
    int_obj->vf = (float)(int_obj->vi);
    break;

  default:
    todo_impl;
  }

  return int_obj;
}

// add
static mt_object* add_object(mt_object* left, mt_object* right) {

  //
  // どっちも数値型
  if (mt_obj_is_numeric(left) && mt_obj_is_numeric(right)) {
    //
    // どちらかが Float であれば、両方 Float にする
    if (left->typeinfo.kind == TYPE_FLOAT)
      right = to_float(right);
    else if (right->typeinfo.kind == TYPE_FLOAT)
      left = to_float(left);

    assert(left->typeinfo.kind == right->typeinfo.kind);

    switch (left->typeinfo.kind) {
    case TYPE_INT:
      left->vi += right->vi;
      break;

    case TYPE_FLOAT:
      left->vf += right->vf;
      break;
    }
  }

  //
  // str + str
  else if (IS_STRING(left) && IS_STRING(right)) {
    vector_append_vector(left->vs, right->vs);
  }

  else {
    todo_impl;
  }

  return left;
}

static mt_object* mul_object(mt_object* left, mt_object* right) {

  //
  // str * int
  if (IS_STRING(left) && IS_INT(right)) {
  _label_mul_str_int:
    mt_object* strobj = mt_obj_new_string();

    for (int i = 0; i < right->vi; i++)
      vector_append_vector(strobj->vs, left->vs);

    return strobj;
  }

  //
  // int * str ( --> swap and goto str*int )
  else if (IS_INT(left) && IS_STRING(right)) {
    swap(left, right);
    goto _label_mul_str_int;
  }

  return left;
}

static mt_object* evaluate(mt_node* node) {
  typedef mt_object* (*expr_fn_ptr_t)(mt_object*, mt_object*);

  static int* case_labels[] = {
      [ND_VALUE] = &&case_value,
      [ND_PROGRAM] = &&case_program,
      [ND_VARDEF] = &&case_vardef,
      [ND_BLOCK] = &&case_block,
  };

  static expr_fn_ptr_t op_expr_labels[] = {
      [ND_MUL] = mul_object,
      [ND_ADD] = add_object,
  };

  mt_object* result = NULL;

  // alertfmt("%hhd\n", node->kind);

  if (node->kind >= _NDKIND_BEGIN_OF_LR_OP_EXPR_ &&
      node->kind <= _NDKIND_END_OF_LR_OP_EXPR_)
    goto case_lr_operator_expr;

  debug(assert(case_labels[node->kind] != 0));

  goto* case_labels[node->kind];

case_value:
  return node->value;

case_program:
  for (size_t i = 0; i < node->child->count; i++)
    result = evaluate(nd_get_child(node, i));

  return result;

case_vardef:
case_block:
  return NULL;

case_lr_operator_expr:
  debug(assert(op_expr_labels[node->kind] != 0));

  return op_expr_labels[node->kind](evaluate(nd_lhs(node)),
                                    evaluate(nd_rhs(node)));
}

void mt_eval_init(void) {
}

void mt_eval_exit(void) {
}

mt_object* mt_eval_evalfull(mt_node* node) {
  return evaluate(node);
}
