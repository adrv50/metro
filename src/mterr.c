#include "mterr.h"
#include "metro.h"

static mt_error _err_top;
static mt_error* errptr;

void metro_init() {
  errptr = &_err_top;
}

void metro_exit() {
  // mt_error のメモリは解放しない
  // エラーが発生した時点でアプリケーションが終了することが予測できます

  // 警告だけを出して続行するならば、厳しくエラーを出して終了したい
}

mt_error* mt_new_error(mt_err_kind_t kind, char const* msg,
                       size_t pos) {
  mt_error* err = calloc(1, sizeof(mt_error));

  err->kind = kind;
  err->msg = msg;

  err->position = pos;

  errptr->_next = err;
  errptr = errptr->_next;

  return err;
}

mt_error* mt_new_error_from_token(mt_err_kind_t kind, char const* msg,
                                  mt_token* token) {
  mt_error* err = mt_new_error(kind, msg, 0);

  err->token = token;

  return err;
}

mt_error* mt_new_error_from_node(mt_err_kind_t kind, char const* msg,
                                 mt_node* node) {
  mt_error* err = mt_new_error(kind, msg, 0);

  err->node = node;

  return err;
}

void mt_error_emit(mt_error* err) {
  size_t pos;
  size_t len;

  (void)len;

  // node の実装がめんどいのでとりあえず token
  // からつくられたことにする
  if (err->node)
    err->token = err->node->tok;

  if (err->token) {
    pos = err->token->pos;
    len = err->token->len;
  }
  else {
    todo_impl;
  }

  printf("error! '%s' pos = %zu\n", err->msg, pos);
}

void mt_abort_with(mt_error* err) {
  mt_error_emit(err);
  exit(1);
}