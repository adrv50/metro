#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "eval.h"
#include "vector.h"

typedef struct {
  node_t* cur;

} eval_context;

static vector* ev_ctx_stack; // vector<eval_context>
static eval_context* ctx;

static void ev_enter(node_t* node) {
  eval_context context = {0};

  context.cur = node;

  ctx = vector_append(ev_ctx_stack, &context);
}

static void ev_leave() {
  vector_pop_back(ev_ctx_stack);

  ctx = vector_last(ev_ctx_stack);
}

static mt_object* ev_eval() {
  int* case_labels[] = {
      [ND_VALUE] = &&case_value,

      [ND_MUL] = &&case_mul,
      [ND_ADD] = &&case_add,
  };

  node_t* node = ctx->cur;
  node_t* lhs = NULL;
  node_t* rhs = NULL;

  if (node->kind >= ND_MUL && node->kind <= ND_ASSIGN) {
    lhs = nd_lhs(node);
    rhs = nd_rhs(node);
  }

  goto* case_labels[node->kind];

case_value:
  return node->value;

case_mul:

case_add:

  return NULL;
}

void mt_eval_init(void) { ev_ctx_stack = vector_new(sizeof(eval_context)); }

void mt_eval_exit(void) { vector_free(ev_ctx_stack); }

mt_object* mt_eval_evalfull(node_t* node) {
  ev_enter(node);

  return ev_eval();
}
