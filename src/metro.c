#include <string.h>
#include "metro.h"

/*

・なぜこうするのか？　何を思ってこうした？
　その瞬間思ったことならば　それをコメントに書こう

・コメントは、未来の自分に宛てた手紙

*/

static mt_error     _err_top;
static mt_error*    errptr;

static source_t*    cur_source;

mt_error*
mt_new_error(mt_err_kind_t kind, char const* msg, size_t pos) {
  mt_error* err = calloc(1, sizeof(mt_error));

  err->kind   = kind;
  err->msg    = msg;

  err->position  = pos;

  errptr->_next = err;
  errptr = errptr->_next;

  return err;
}

mt_error*
mt_new_error_from_token(mt_err_kind_t kind, char const* msg, token_t* token) {
  mt_error* err = mt_new_error(kind, msg, NULL);

  err->token = token;

  return err;
}

mt_error*
mt_new_error_from_node(mt_err_kind_t kind, char const* msg, node_t* node) {
  mt_error* err = mt_new_error(kind, msg, NULL);

  err->node = node;

  return err;
}

void  mt_error_emit(mt_error* err) {
  size_t pos;
  size_t len;

  // めんどいのでとりあえず token からつくられたことにする
  err->token = err->node->tok;

  if( err->token ) {
    pos = err->token->pos;
    len = err->token->len;
  }

  printf("error! '%s' pos = %zu\n", err->msg, pos);


}

void  mt_abort_with(mt_error* err) {
  mt_error_emit(err);
  exit(1);
}

mtdriver* driver_new(char* path) {
  mtdriver* dr = calloc(1, sizeof(mtdriver));

  return dr;
}

void driver_free(mtdriver* dr) {
  // todo: free lexer

  free(dr);
}

source_t* driver_get_current_source(mtdriver* dr) {

}

void  metro_init() {
  errptr = _err_top._next;
}

void  metro_exit() {
  
}
