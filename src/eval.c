#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "utils.h"
#include "alert.h"
#include "eval.h"
#include "vector.h"
#include "utf-convert.h"

typedef struct {
  mt_node* decl; // ptr to node of let-statement => ND_VARDEF
  char const* name;
  mt_object* value;
} lvar_data_t;

typedef struct {
  mt_node* block;  // ND_BLOCK
  vector* varlist; // vector<lvar_data_t>
} variable_list_t;

typedef struct {
  vector* varlist_stack; // vector<variable_list_t>
} eval_context_t;

static eval_context_t g_context;

static mt_object* evaluate(mt_node* node);

static variable_list_t* mv_ev_ctx_get_current_varlist() {
  return (variable_list_t*)vector_last(g_context.varlist_stack);
}

//
// create a new lvar_data_t from node (ND_VARDEF)
static lvar_data_t* mt_ev_vardef_wrap(mt_node* nd) {
  debug(assert(nd->kind == ND_VARDEF));

  lvar_data_t* vd = calloc(1, sizeof(lvar_data_t*));

  vd->decl = nd;
  vd->name = nd->name;

  if (nd_let_init(nd)) {
    vd->value = evaluate(nd_let_init(nd));
  }

  return vd;
}

static void mt_ev_enter_block(mt_node* block) {
  variable_list_t* p_vl = vector_append(
      g_context.varlist_stack, calloc(1, sizeof(variable_list_t)));

  p_vl->block = block;
  p_vl->varlist = vector_new(sizeof(lvar_data_t*));
}

static void mt_ev_leave_block(mt_node* block) {
  variable_list_t* cur_VL = mv_ev_ctx_get_current_varlist();

  assert(cur_VL->block == block);

  vector_free(cur_VL->varlist);
  vector_pop_back(g_context.varlist_stack);
}

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

  int_obj->typeinfo.kind = TYPE_FLOAT;

  return int_obj;
}

//
// -------------------------
//  ND_MUL
// -------------------------
//
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

  else {
    todo_impl;
  }

  return left;
}

//
// -------------------------
//  ND_DIV
// -------------------------
//
static mt_object* div_object(mt_object* left, mt_object* right) {

  switch (left->typeinfo.kind) {
  case TYPE_FLOAT:
    if (IS_FLOAT(right))
      left->vf /= right->vf;
    else if (IS_INT(right))
      left->vf /= (float)right->vi;
    else
      todo_impl;

    break;

  case TYPE_INT:
    if (IS_FLOAT(right))
      left->vi /= (int)right->vf;
    else if (IS_INT(right))
      left->vi /= right->vi;
    else
      todo_impl;

    break;

  default:
    todo_impl;
  }

  return left;
}

//
// -------------------------
//  ND_MOD
// -------------------------
//
static mt_object* mod_object(mt_object* left, mt_object* right) {

  debug(assert(IS_INT(left) && IS_INT(right)));

  left->vi %= right->vi;

  return left;
}

//
// -------------------------
//  ND_ADD
// -------------------------
//
static mt_object* add_object(mt_object* left, mt_object* right) {

  //
  // どっちも数値型
  if (mt_obj_is_numeric(left) && mt_obj_is_numeric(right)) {
    //
    // どちらかが Float であれば、両方 Float にする
    if (left->typeinfo.kind == TYPE_FLOAT) {
      if (right->typeinfo.kind != TYPE_FLOAT)
        right = to_float(right);
    }
    else if (right->typeinfo.kind == TYPE_FLOAT) {
      if (left->typeinfo.kind != TYPE_FLOAT)
        left = to_float(left);
    }

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

//
// -------------------------
//  ND_SUB
// -------------------------
//
static mt_object* sub_object(mt_object* left, mt_object* right) {

  //
  // どっちも数値型
  if (mt_obj_is_numeric(left) && mt_obj_is_numeric(right)) {
    //
    // どちらかが Float であれば、両方 Float にする
    if (left->typeinfo.kind == TYPE_FLOAT) {
      if (right->typeinfo.kind != TYPE_FLOAT)
        right = to_float(right);
    }
    else if (right->typeinfo.kind == TYPE_FLOAT) {
      if (left->typeinfo.kind != TYPE_FLOAT)
        left = to_float(left);
    }

    assert(left->typeinfo.kind == right->typeinfo.kind);

    switch (left->typeinfo.kind) {
    case TYPE_INT:
      left->vi -= right->vi;
      break;

    case TYPE_FLOAT:
      left->vf -= right->vf;
      break;
    }
  }

  else {
    todo_impl;
  }

  return left;
}

//
// ============================================
//  Evaluate a nodes
// ============================================
//
static mt_object* evaluate(mt_node* node) {
  typedef mt_object* (*expr_fp_t)(mt_object*, mt_object*);

  // clang-format off
  static int* case_labels[] = {
    [ND_VALUE]      = &&case_value,
    [ND_VARIABLE]   = &&case_variable,
    [ND_CALLFUNC]   = &&case_callfunc,

    [ND_ASSIGN]     = &&case_assign,

    [ND_VARDEF]     = &&case_vardef,
    [ND_BLOCK]      = &&case_block,

    [ND_FUNCTION]   = &&case_skip,
    [ND_ENUM]       = &&case_skip,
    [ND_STRUCT]     = &&case_skip,

    [ND_PROGRAM]    = &&case_program,
  };

  static expr_fp_t op_expr_labels[] = {
    [ND_MUL] = mul_object,
    [ND_DIV] = div_object,
    [ND_MOD] = mod_object,
    [ND_ADD] = add_object,
    [ND_SUB] = sub_object,
  };
  // clang-format on

  if (node->kind >= _NDKIND_BEGIN_OF_LR_OP_EXPR_ &&
      node->kind <= _NDKIND_END_OF_LR_OP_EXPR_)
    goto case_lr_operator_expr;

  debug(assert(case_labels[node->kind] != 0));

  goto* case_labels[node->kind];

case_value:
  return node->value;

case_variable:
  todo_impl;

//
// ND_CALLFUNC
//
case_callfunc : {
  mt_object* args[node->child->count];

  for (size_t i = 0; i < node->child->count; i++)
    args[i] = evaluate(nd_get_child(node, i));

  //
  // println
  if (node->len >= 7 && strncmp(node->name, "println", 7) == 0) {
    for (size_t i = 0; i < node->child->count; i++)
      print_object(args[i]);

    puts("");

    return NULL;
  }

  todo_impl;
}

case_assign:
  todo_impl;

case_block:
case_program : {
  mt_object* res;

  for (size_t i = 0; i < node->child->count; i++)
    res = evaluate(nd_get_child(node, i));

  return res;
}

case_vardef:
  return NULL;

case_skip:
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
