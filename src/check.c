#include <stdio.h>
#include <string.h>
#include <assert.h>

#include "alert.h"
#include "metro.h"
#include "mterr.h"

#include "check.h"
#include "builtin.h"

typedef struct {
  mt_node* node;
  mt_type_info type;

  mt_node* callee;
  mt_builtin_func_t const* callee_builtin;

} check_result_t;

static check_result_t check(mt_node* node);

static inline bool is_contain(mt_type_kind kind, mt_type_kind a,
                              mt_type_kind b) {
  return a == kind || b == kind;
}

static inline bool is_numeric(mt_type_kind k) {
  return k == TYPE_INT || k == TYPE_FLOAT;
}

struct block_context_t;
typedef struct block_context_t block_context_t;

typedef struct {
  mt_node* decl;

  char const* name;
  int namelen;

  int depth;
  int index;

  mt_type_info type;
  bool is_type_deducted;

} emu_lvar_t;

typedef struct {
  struct block_context_t* block;

  vector* list; // vector<emu_lvar_t>

} emu_varlist_t;

static emu_varlist_t* new_emuvlist(struct block_context_t* blk,
                                   vector* list) {
  emu_varlist_t* vl = calloc(1, sizeof(emu_varlist_t));

  vl->block = blk;
  vl->list = list;

  return vl;
}

//
//  fn: emu_varlist_append_variable
//
//  params:
//    vlist         = inst of `emu_varlist_t` to append new lvar
//    name, len     = name
//    type          = type of variable
//
static emu_lvar_t*
emu_varlist_append_variable(emu_varlist_t* vlist, char const* name,
                            int len, mt_type_info type,
                            bool is_type_deducted) {
  emu_lvar_t lvar = {0};

  lvar.name = name;
  lvar.namelen = len;

  lvar.type = type;
  lvar.is_type_deducted = is_type_deducted;

  return (emu_lvar_t*)vector_append(vlist->list, &lvar);
}

struct block_context_t {
  mt_node* block;

  emu_varlist_t* vlist;

  vector* functions; // vector<mt_node*>
};

static vector* blockctx_stack; // vector<block_context_t>

static block_context_t* ck_enter_block(mt_node* blk) {
  block_context_t tmp = {0};
  block_context_t* ctx = vector_append(blockctx_stack, &tmp);

  ctx->block = blk;
  ctx->vlist = new_emuvlist(ctx, vector_new(sizeof(emu_lvar_t)));

  ctx->functions = vector_new(sizeof(mt_node*));

  for (int i = 0; i < blk->child->count; i++) {
    switch (nd_get_child(blk, i)->kind) {
    case ND_FUNCTION:
      vector_append(ctx->functions, &nd_get_child(blk, i));
      break;
    }
  }

  return ctx;
}

static block_context_t* ck_get_cur_block_ctx() {
  return vector_last(blockctx_stack);
}

static void ck_leave_block() {
  block_context_t* blk = ck_get_cur_block_ctx();

  vector_free(blk->vlist->list);
  free(blk->vlist);

  vector_pop_back(blockctx_stack);
}

static emu_lvar_t* ck_define_variable(mt_node* letnode) {
  assert(letnode->kind == ND_VARDEF);

  emu_varlist_t* cur_vlist = ck_get_cur_block_ctx()->vlist;
  emu_lvar_t* pv = NULL;

  for (int i = 0; i < cur_vlist->list->count; i++) {
    emu_lvar_t* var = (emu_lvar_t*)vector_get(cur_vlist->list, i);

    if (var->namelen == letnode->len &&
        strncmp(var->name, letnode->name, letnode->len) == 0) {
      pv = var;
      break;
    }
  }

  if (!pv) {
    pv = emu_varlist_append_variable(
        cur_vlist, letnode->name, letnode->len,
        (mt_type_info){TYPE_NONE}, false);
  }

  pv->depth = letnode->id_info.vdepth = blockctx_stack->count - 1;
  pv->index = letnode->id_info.index = cur_vlist->list->count - 1;

  mt_node* ndtype = nd_let_type(letnode);
  mt_node* ndinit = nd_let_init(letnode);

  mt_type_info tptype, tpinit;

  if (ndtype)
    tptype = check(ndtype).type;

  if (ndinit)
    tpinit = check(ndinit).type;

  if (!ndtype && !ndinit)
    return pv;
  else
    pv->is_type_deducted = true;

  if (ndinit) {
    if (ndtype) {
      if (!mt_type_is_equal(tptype, tpinit)) {
        mt_add_error_from_node(ERR_TYPE_MISMATCH, "type mismatch",
                               ndinit);
      }
    }

    pv->type = tpinit;
  }
  else if (ndtype) {
    pv->type = tptype;
  }

  return pv;
}

static emu_lvar_t* ck_find_variable(char const* name, int len) {
  for (int i = blockctx_stack->count - 1; i >= 0; i--) {
    block_context_t* ctx = vector_get(blockctx_stack, i);

    for (int j = 0; j < ctx->vlist->list->count; j++) {
      emu_lvar_t* lvar = vector_get(ctx->vlist->list, j);

      if (lvar->namelen == len && !strncmp(lvar->name, name, len))
        return lvar;
    }
  }

  return NULL;
}

typedef enum {
  ID_VARIABLE,

  ID_FUNCTION,

  ID_ENUM,
  ID_STRUCT,
  ID_CLASS,

  ID_NAMESPACE,

  // builtin
  ID_BLT_FUNC,

  ID_UNKNOWN,

} identifier_kind;

typedef struct {
  identifier_kind kind;

  mt_node* nd;

  emu_lvar_t* p_lvar; // if variable

  mt_builtin_func_t const* builtin_func;

} identifier_info_t;

mt_node* find_name_in_node(mt_node* node, char const* name, int len) {
  /*
  switch (node->kind) {
  case ND_ENUM:
  case ND_STRUCT:
  case ND_CLASS:
  case ND_NAMESPACE:
    break;

  default:
    alertmsg(called find_name_in_node() but node is not enum or
             struct or class or namespace.);
  }
  */

  for (int i = 0; i < node->child->count; i++) {
    mt_node* nd = nd_get_child(node, i);

    switch (nd->kind) {
    case ND_ENUM:
    case ND_STRUCT:
    case ND_CLASS:
    case ND_NAMESPACE:
    case ND_VARDEF:
    case ND_FUNCTION:
      if (nd->len == len && strncmp(nd->name, name, len) == 0)
        return nd;
    }
  }

  return NULL;
}

//
//  ck_get_identififer_info()
//
//  識別子から関数、変数などを探す
//
identifier_info_t ck_get_identififer_info(mt_node* node) {
  identifier_info_t idinfo = {0};

  idinfo.kind = ID_UNKNOWN;

  // scope resolution operator "::"
  if (node->kind == ND_SCOPE_RESOLUTION) {
    if (nd_rhs(node)->kind != ND_IDENTIFIER) {
      alert;
      mt_add_error_from_token(ERR_INVALID_SYNTAX, "invalid syntax",
                              nd_rhs(node)->tok);
    }

    // get info of left side
    identifier_info_t leftinfo =
        ck_get_identififer_info(nd_lhs(node));

    switch (leftinfo.kind) {
    case ID_ENUM:
    case ID_STRUCT:
    case ID_CLASS:
    case ID_NAMESPACE:
      break;

    default:
      mt_add_error_from_token(ERR_INVALID_SYNTAX, "", node->tok);
    }

    mt_node* found = find_name_in_node(
        leftinfo.nd, nd_rhs(node)->name, nd_rhs(node)->len);

    if (!found) {
      mt_add_error_from_token_with_fmt(
          ERR_CANNOT_FIND_NAME, nd_rhs(node)->tok,
          "'%s' is not named in '%s'", nd_rhs(node)->tok,
          "ABCDEF12345");
    }

    idinfo.nd = found;
  }
  else {

    //
    // find variable
    if ((idinfo.p_lvar = ck_find_variable(node->name, node->len))) {
      idinfo.kind = ID_VARIABLE;
      idinfo.nd = idinfo.p_lvar->decl;

      return idinfo;
    }

    //
    // find builtin function
    mt_builtin_func_t const* bfun = mt_get_builtin_functions();

    for (int i = 0; i < mt_get_builtin_functions_count();
         i++, bfun++) {

      if (bfun->namelen == node->len &&
          !strncmp(bfun->name, node->name, node->len)) {

        idinfo.kind = ID_BLT_FUNC;
        idinfo.builtin_func = bfun;

        return idinfo;
      }
    }

    for (int i = (int)blockctx_stack->count - 1; i >= 0; i--) {
      alert;

      block_context_t* ctx =
          (block_context_t*)vector_get(blockctx_stack, i);

      mt_node* ndfind =
          find_name_in_node(ctx->block, node->name, node->len);

      if (ndfind) {
        idinfo.nd = ndfind;
        break;
      }
    }
  }

  if (!idinfo.nd) {
    mt_add_error_from_token(ERR_CANNOT_FIND_NAME, "no name found",
                            node->tok);

    mt_error_emit_and_exit();
  }

  switch (idinfo.nd->kind) {
  case ND_VARDEF:
    idinfo.kind = ID_VARIABLE;

    break;

  case ND_FUNCTION:
    idinfo.kind = ID_FUNCTION;
    break;

  default:
    todo_impl;
  }

  return idinfo;
}

static mt_ck_checked_log_t* ck_create_log(mt_node* node) {
  return (node->checked = calloc(1, sizeof(mt_ck_checked_log_t)));
}

static check_result_t check(mt_node* node) {

  if (node->kind > _NDKIND_BEGIN_OF_LR_OP_EXPR_ &&
      node->kind < _NDKIND_END_OF_LR_OP_EXPR_) {

    check_result_t left = check(nd_lhs(node)),
                   right = check(nd_rhs(node));

    return left;
  }

  check_result_t result = {0};

  result.node = node;

  switch (node->kind) {

  case ND_TYPENAME:

    break;

  case ND_VALUE:
    assert(node->value);
    break;

  case ND_IDENTIFIER:
  case ND_SCOPE_RESOLUTION: {
    identifier_info_t idinfo = ck_get_identififer_info(node);

    switch (idinfo.kind) {
    // Variable
    case ID_VARIABLE: {
      // alert;
      // printf("%.*s\n", idinfo.nd->len, idinfo.nd->name);

      node->id_info.kind = NID_VARIABLE;

      if (!idinfo.p_lvar->is_type_deducted) {
        mt_add_error_from_token(ERR_USE_BEFORE_TYPE_DEDUCTION,
                                "use variable before type deduction",
                                node->tok);
        mt_error_emit_and_exit();
      }

      node->id_info.vdepth = idinfo.p_lvar->depth;
      node->id_info.index = idinfo.p_lvar->index;

      result.type = idinfo.p_lvar->type;
      break;
    }

    // Built-in Function
    case ID_BLT_FUNC: {
      node->id_info.kind = NID_BLT_FUNC;
      node->id_info.callee_builtin = idinfo.builtin_func;

      result.type = mt_type_info_new(TYPE_FUNCTION);
      result.callee_builtin = idinfo.builtin_func;

      break;
    }

    default:
      todo_impl;
    }

    node->id_info.is_valid = true;

    break;
  }

  case ND_CALLFUNC: {

    check_result_t fun = check(nd_lhs(node));

    if (fun.type.kind != TYPE_FUNCTION) {
      mt_add_error_from_token(ERR_TRY_TO_CALL_NOT_CALLABLE,
                              "not callable", node->tok);
    }

    for (int i = 0; i < node->child->count; i++) {
      check(nd_get_child(node, i));
    }

    if (fun.callee_builtin) {
      result.type = fun.callee_builtin->return_type;
      break;
    }

    todo_impl;
    break;
  }

  case ND_VARDEF: {
    ck_define_variable(node);

    break;
  }

  case ND_BLOCK: {
    ck_enter_block(node);

    for (size_t i = 0; i < node->child->count; i++) {
      check(nd_get_child(node, i));
    }

    ck_leave_block();
    break;
  }
  }

  return result;
}

// ===============================
//    Checker
// ===============================
void mt_ck_check(mt_node* node) {

  blockctx_stack = vector_new(sizeof(block_context_t));

  check(node);

  vector_free(blockctx_stack);
}