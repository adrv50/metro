#include "mterr.h"
#include "metro.h"

static mt_error _err_top;
static mt_error* errptr = &_err_top;
static size_t err_count = 0;

void mt_err_init() {
}

void mt_err_exit() {
}

mt_error* mt_add_error(mt_err_kind_t kind, char const* msg,
                       size_t pos) {
  mt_error* err = calloc(1, sizeof(mt_error));

  err->kind = kind;
  err->msg = msg;

  err->position = pos;

  // err->_next = errptr->_next;

  errptr->_next = err;
  errptr = err;

  err_count++;

  return err;
}

mt_error* mt_add_error_from_token(mt_err_kind_t kind, char const* msg,
                                  mt_token* token) {
  mt_error* err = mt_add_error(kind, msg, 0);

  err->token = token;

  return err;
}

mt_error* mt_add_error_from_node(mt_err_kind_t kind, char const* msg,
                                 mt_node* node) {
  mt_error* err = mt_add_error(kind, msg, 0);

  err->node = node;

  return err;
}

void mt_error_emit(mt_error* err) {
  size_t pos = 0, len = 0;

  (void)len;

  source_file* src =
      mt_driver_get_current_source(mt_driver_get_cur_instance());

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

  // trim line
  int linenum = 1;
  int beginpos = 0;
  int endpos = (int)src->length;

  char const* line = NULL;
  int line_len = 0;

  for (int i = 0; i < pos; i++) {
    if (src->data[i] == '\n') {
      beginpos = i + 1;
      linenum++;
    }
  }

  for (int i = beginpos; i < endpos; i++) {
    if (src->data[i] == '\n') {
      endpos = i;
    }
  }

  int pos_on_line = pos - beginpos - 1;

  line = src->data + beginpos;
  line_len = endpos - beginpos;

  bool is_warn = false; /* feature, TODO */

  printf(COL_BOLD COL_WHITE "%s:%d:%d: %s " COL_WHITE "%s\n",
         src->path, linenum, pos_on_line,
         is_warn ? (COL_MAGENTA "warning:") : (COL_RED "error:"),
         err->msg);

  printf("     |\n");
  printf(" % 3d | %.*s\n", linenum, line_len, line);

  printf("     | ");
  for (int i = 1; i < pos_on_line; i++)
    printf(" ");

  printf("^\n     |\n");
}

void mt_error_emit_and_exit() {
  for (mt_error* ep = _err_top._next; ep; ep = ep->_next) {
    mt_error_emit(ep);
  }

  exit(1);
}

void mt_error_check() {
  if (err_count == 0)
    return;

  mt_error_emit_and_exit();
}