#include <stdio.h>
#include <string.h>
#include <assert.h>

#include "alert.h"
#include "check.h"
#include "metro.h"

#include "mterr.h"

static mt_type_info check(mt_node* node);

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
};

static vector* blockctx_stack; // vector<block_context_t>

static block_context_t* ck_enter_block(mt_node* blk) {
  block_context_t temp = {0};
  block_context_t* ctx = vector_append(blockctx_stack, &temp);

  ctx->block = blk;
  ctx->vlist = new_emuvlist(ctx, vector_new(sizeof(emu_lvar_t)));

  return ctx;
}

static block_context_t* ck_get_cur_block_ctx() {
  return vector_last(blockctx_stack);
}

static void ck_leave_block() {
  block_context_t* blk = ck_get_cur_block_ctx();

  vector_free(blk->vlist->list);

  vector_pop_back(blockctx_stack);
}

static void ck_find_variable(mt_node* node) {
  assert(node->kind == ND_IDENTIFIER || node->kind ||
         ND_SCOPE_RESOLUTION);
}

static void ck_define_variable(mt_node* node) {
  assert(node->kind == ND_VARDEF);

  emu_varlist_t* cur_vlist = ck_get_cur_block_ctx()->vlist;
  emu_lvar_t* pv = NULL;

  for (int i = 0; i < cur_vlist->list->count; i++) {
    emu_lvar_t* var = (emu_lvar_t*)vector_get(cur_vlist->list, i);

    if (var->namelen == node->len &&
        strncmp(var->name, node->name, node->len) == 0) {
      pv = var;
      break;
    }
  }

  if (!pv) {
    pv =
        emu_varlist_append_variable(cur_vlist, node->name, node->len,
                                    (mt_type_info){TYPE_NONE}, false);
  }

  mt_node* ndtype = nd_let_type(node);
  mt_node* ndinit = nd_let_init(node);

  mt_type_info tptype, tpinit;

  if (ndtype && ndinit) {
    tptype = check(ndtype);
    tpinit = check(ndinit);

    if (!mt_type_is_equal(tptype, tpinit)) {
    }
  }
}

static mt_type_info check(mt_node* node) {

  switch (node->kind) {
  case ND_VALUE:
    assert(node->value);
    break;

  case ND_IDENTIFIER: {
    alert;

    break;
  }

  case ND_SCOPE_RESOLUTION: {
    alert;

    break;
  }

  case ND_CALLFUNC: {

    alert;

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

  return mt_type_info_new(TYPE_NONE);
}

// ===============================
//    Checker
// ===============================
void mt_ck_check(mt_node* node) {

  blockctx_stack = vector_new(sizeof(block_context_t));

  check(node);

  vector_free(blockctx_stack);
}