#include <string.h>
#include "parser.h"
#include "eval.h"
#include "check.h"
#include "metro.h"
#include "mterr.h"

static vector* _instances;

mt_driver* mt_driver_new(char* path) {
  mt_driver* dr = calloc(1, sizeof(mt_driver));

  vector_append(_instances, &dr);

  dr->source = source_file_new(path);

  return dr;
}

void mt_driver_free(mt_driver* dr) {
  // todo: free lexer

  vector_pop_back(_instances);

  free(dr);
}

mt_driver* mt_driver_get_cur_instance() {
  return *(mt_driver**)(vector_last(_instances));
}

// debug
static void print_int_vector(vector* v) {
  for (size_t i = 0; i < v->count; i++)
    printf("%d ", *(int*)vector_get(v, i));

  printf("\n");
}

// debug
__attribute__((unused)) static void test1() {
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
void print_token(mt_token* tok) {
  printf("{ ");

  while (tok && tok->kind != TOK_END) {
    printf("%.*s ", (int)tok->len, tok->str);

    if (tok->next)
      tok = tok->next;
  }

  printf("}\n");
}

int mt_driver_main(mt_driver* dr, int argc, char** argv) {
  (void)argc;
  (void)argv;

  dr->lexer = lexer_new(dr->source);

  mt_token* tok = lexer_lex(dr->lexer);
  mt_error_check();

  mt_node* nd = parser_parse(dr->source, tok);

  mt_error_check();

  // check
  mt_ck_check(nd);
  mt_error_check();

  mt_eval_init();

  mt_eval_evalfull(nd);

  mt_eval_exit();

  node_free(nd);
  metro_exit();

  return 0;
}

source_file* mt_driver_get_current_source(mt_driver* dr) {
  return dr->source;
}

void metro_init() {
  mt_err_init();

  _instances = vector_new(sizeof(mt_driver*));
}

void metro_exit() {
  mt_err_exit();

  vector_free(_instances);
}
