#include <string.h>
#include "metro.h"

/*
・なぜこうするのか？　何を思ってこうした？
　その瞬間思ったことならば　それをコメントに書こう

・コメントは、未来の自分に宛てた手紙
*/

static mt_error _err_top;
static mt_error* errptr;

static source_t* cur_source;

mt_error* mt_new_error(mt_err_kind_t kind, char const* msg, size_t pos) {
  mt_error* err = calloc(1, sizeof(mt_error));

  err->kind = kind;
  err->msg = msg;

  err->position = pos;

  errptr->_next = err;
  errptr = errptr->_next;

  return err;
}

mt_error* mt_new_error_from_token(mt_err_kind_t kind, char const* msg,
                                  token_t* token) {
  mt_error* err = mt_new_error(kind, msg, 0);

  err->token = token;

  return err;
}

mt_error* mt_new_error_from_node(mt_err_kind_t kind, char const* msg,
                                 mt_node_t* node) {
  mt_error* err = mt_new_error(kind, msg, 0);

  err->node = node;

  return err;
}

void mt_error_emit(mt_error* err) {
  size_t pos;
  size_t len;

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

mtdriver* driver_new(char* path) {
  mtdriver* dr = calloc(1, sizeof(mtdriver));

  dr->source = source_new(path);

  return dr;
}

void driver_free(mtdriver* dr) {
  // todo: free lexer

  free(dr);
}

// debug
static void print_int_vector(vector* v) {
  for (size_t i = 0; i < v->count; i++)
    printf("%d ", *(int*)vector_get(v, i));

  printf("\n");
}

// debug
static void test1() {
  // char buf[] = "abcdefghijklmn";

  // memcpyex(buf + 4, buf + 2, 3);
  // printf("%s\n",buf);

  // return;

  vector* vec = vector_new(sizeof(int));

  for (int i = 10; i < 20; i++)
    vector_append(vec, &i);

  vector* vec2 = vector_new(sizeof(int));

  for (int i = 1; i < 5; i++)
    vector_append(vec2, &i);

  vector_insert_vector(vec, 3, vec2);

  vector_erase_range(vec, 3, 7);

  print_int_vector(vec);
}

//
// view all token
//
void print_token(token_t* tok) {
  printf("{ ");

  while (tok && tok->kind != TOK_END) {
    printf("%.*s ", (int)tok->len, tok->str);

    if (tok->next)
      tok = tok->next;
  }

  printf("}\n");
}

int driver_main(mtdriver* dr, int argc, char** argv) {
  (void)argc;
  (void)argv;

  metro_init();

  dr->lexer = lexer_new(dr->source);

  token_t* tok = lexer_lex(dr->lexer);

  // mt_abort_with(mt_new_error_from_token(ERR_INVALID_TOKEN, "test
  // error", tok));

  // print_token(tok);

  mt_node_t* nd = parser_parse(dr->source, tok);

  // print_node(nd);
  // puts("\n");

  mt_eval_init();

  mt_object* result = mt_eval_evalfull(nd);

  // printf("%ld\n", result->vi);

  print_object(result);
  puts("");

  // compiler_compile_full(nd);

  node_free(nd);
  metro_exit();

  return 0;
}

source_t* driver_get_current_source(mtdriver* dr) {
  return cur_source;
}

void metro_init() {
  errptr = &_err_top;
}

void metro_exit() {
  // mt_error のメモリは解放しない
  // エラーが発生した時点でアプリケーションが終了することが予測できます

  // 警告だけを出して続行するならば、厳しくエラーを出して終了したい
}
