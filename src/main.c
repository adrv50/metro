#include <stdio.h>

#include "alert.h"
#include "vector.h"

#include "lexer.h"
#include "parser.h"
#include "eval.h"

#include "metro.h"

static void print_int_vector(vector* v) {
  for(size_t i=0;i<v->count;i++)
    printf("%d ", *(int*)vector_get(v, i));

  printf("\n");
}

void test1() {

  // char buf[] = "abcdefghijklmn";

  // memcpyex(buf + 4, buf + 2, 3);
  // printf("%s\n",buf);

  // return;

  vector* vec = vector_new(sizeof(int));

  for(int i=10;i<20;i++)
    vector_append(vec, &i);

  vector* vec2 = vector_new(sizeof(int));

  for(int i=1;i<5;i++)
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

  while( tok ) {
    printf("%.*s ", (int)tok->len, tok->str);

    if( tok->next )
      tok = tok->next;
    else {
      printf("}\n");
      break;
    }
  }
}

int main(int argc, char** argv) {

  (void)argc;
  (void)argv;

  metro_init();

  mtlexer*  lexer = lexer_new("test.metro");

  token_t*  tok = lexer_lex(lexer);

  print_token(tok);

  node_t* nd = parser_parse(lexer->src, tok);

  print_node(nd);

  evaluator_init(nd);
  mt_object_t* result = evaluator_evalnode(nd);

  node_free(nd);

  metro_exit();

  return 0;
}