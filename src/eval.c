#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "utils.h"
#include "alert.h"
#include "vector.h"
#include "utf-convert.h"

#include "mterr.h"
#include "metro.h"
#include "eval.h"

//
//  struct lvar_data_t:
//
//  for keep variable in variable_list_t
//
typedef struct {
  mt_node* decl; // ptr to node of let-statement => ND_VARDEF
  char const* name;
  mt_object* value;
} lvar_data_t;

//
//  struct variable_list_t:
//
typedef struct {
  mt_node* block;  // ND_BLOCK (owner of this list)
  vector* varlist; // vector<lvar_data_t>
} variable_list_t;

//
//  struct eval_context_t
//
typedef struct {
  vector* varlist_stack; // vector<variable_list_t>
} eval_context_t;

static eval_context_t g_context; // context

static mt_object* evaluate(mt_node* node);

static variable_list_t* mt_ev_ctx_get_current_varlist() {
  return (variable_list_t*)vector_last(g_context.varlist_stack);
}

static lvar_data_t* get_var_from_index(int depth, int index) {
  return vector_get(vector_get_as(variable_list_t, g_context.varlist_stack, depth).varlist, index);
}

static lvar_data_t* mt_ev_find_variable(char const* name,
                                        size_t len) {
  for (i64 i = g_context.varlist_stack->count - 1; i >= 0; i--) {
    variable_list_t* vl = vector_get(g_context.varlist_stack, i);

    for (size_t j = 0; j < vl->varlist->count; j++) {
      lvar_data_t* vd = vector_get(vl->varlist, j);

      if (strncmp(vd->name, name, len) == 0)
        return vd;
    }
  }

  return NULL;
}

//
//  mt_ev_makever:
//    create variable in current var-list
//
static lvar_data_t* mt_ev_makevar(mt_node* nd) {
  debug(assert(nd->kind == ND_VARDEF));

  variable_list_t* cur_vl = mt_ev_ctx_get_current_varlist();
  lvar_data_t* var = NULL;

  for (size_t i = 0; i < cur_vl->varlist->count; i++) {
    if (strncmp(vector_get_as(lvar_data_t, cur_vl->varlist, i).name,
                nd->name, nd->len) == 0) {
      var = vector_get(cur_vl->varlist, i);
      break;
    }
  }

  if (!var) {
    var = vector_append(cur_vl->varlist,
                        calloc(1, sizeof(lvar_data_t)));

    var->decl = nd;
    var->name = nd->name;
  }

  if (nd_let_init(nd)) {
    var->value = evaluate(nd_let_init(nd));
  }

  return var;
}

//
// enter in block
//
static void mt_ev_enter_block(mt_node* block) {
  variable_list_t* p_vl = vector_append(
      g_context.varlist_stack, calloc(1, sizeof(variable_list_t)));

  p_vl->block = block;
  p_vl->varlist = vector_new(sizeof(lvar_data_t));
}

//
// leave from block
//
static void mt_ev_leave_block(mt_node* block) {
  variable_list_t* cur_VL = mt_ev_ctx_get_current_varlist();

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
//  Evaluator
// ============================================
//
static mt_object* evaluate(mt_node* node) {
  typedef mt_object* (*expr_fp_t)(mt_object*, mt_object*);

  static int* case_labels[] = {
    [ND_VALUE]      = &&case_value,

    [ND_IDENTIFIER]         = &&case_identifier,
    [ND_SCOPE_RESOLUTION]   = &&case_scope_resolution,

    [ND_ASSIGN]     = &&case_assign,

    [ND_VARDEF]     = &&case_vardef,
    [ND_BLOCK]      = &&case_block,

    [ND_FUNCTION]   = &&case_skip,
    [ND_ENUM]       = &&case_skip,
    [ND_STRUCT]     = &&case_skip,
  };

  static expr_fp_t expr_eval_funcs[] = {
    [ND_MUL] = mul_object,
    [ND_DIV] = div_object,
    [ND_MOD] = mod_object,
    [ND_ADD] = add_object,
    [ND_SUB] = sub_object,
  };

  static mt_object* result;

  switch (node->kind) {
  case ND_CALLFUNC: {
    mt_object* callee = evaluate(nd_get_child(node, 0));

    if (!callee || callee->typeinfo.kind != TYPE_FUNCTION) {
      mt_add_error_from_token(ERR_TYPE_MISMATCH,
                              "tried to call not callable object",
                              node->tok);
    }

    if( callee->vfn_builtin ) {
      int argc = node->child->count - 1;
      mt_object** argtable = calloc(argc + 1, sizeof(mt_object));

      for( int i = 0; i < argc; i++ )
        argtable[i] = evaluate(nd_get_child(node, i + 1));

      result = callee->vfn_builtin->impl(argc, argtable);

      free(argtable);
    }
    else {
      todo_impl;
    }

    return result;
  }
  }

  if (node->kind >= _NDKIND_BEGIN_OF_LR_OP_EXPR_ &&
      node->kind <= _NDKIND_END_OF_LR_OP_EXPR_)
    goto case_lr_operator_expr;

  if (!case_labels[node->kind]) {
    alertfmt("evaluator of node kind %d is not implemented",
             node->kind);
    exit(1);
  }
  goto* case_labels[node->kind];

case_value:
  return node->value;

case_identifier:
case_scope_resolution:
  {
    switch( node->id_info.kind ) {
      case NID_VARIABLE:
        return get_var_from_index(node->id_info.vdepth, node->id_info.index)->value;

      case NID_FUNCTION:
        return mt_obj_new_func(node->id_info.callee_nd);

      case NID_BLT_FUNC:
        return mt_obj_new_blt_func(node->id_info.callee_builtin);
    }

    todo_impl;
  }

case_assign:
  todo_impl;

case_block:
  mt_ev_enter_block(node);

  for (size_t i = 0; i < node->child->count; i++)
    result = evaluate(nd_get_child(node, i));

  mt_ev_leave_block(node);

  return result;

case_vardef:
  mt_ev_makevar(node);
  return NULL;

case_skip:
  return NULL;

case_lr_operator_expr:
  if (!expr_eval_funcs[node->kind]) {
    alertfmt("evaluator of node kind %d is not implemented",
             node->kind);
    exit(1);
  }

  return expr_eval_funcs[node->kind](evaluate(nd_lhs(node)),
                                     evaluate(nd_rhs(node)));
}

void mt_eval_init(void) {
  g_context.varlist_stack = vector_new(sizeof(variable_list_t));
}

void mt_eval_exit(void) {
  vector_free(g_context.varlist_stack);
}

mt_object* mt_eval_evalfull(mt_node* node) {
  mt_eval_init();

  mt_object* result = evaluate(node);

  mt_eval_exit();

  return result;
}
