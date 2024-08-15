#pragma once

#include "token.h"
#include "vector.h"

#define   nd_get_child(_ND, _N)    (vector_get((_ND)->child, _N))

#define   nd_lhs(_ND)     nd_get_child(_ND, 0)
#define   nd_rhs(_ND)     nd_get_child(_ND, 1)

typedef u16  node_kind_t;

enum {
  ND_VALUE,
  ND_VARIABLE,

  ND_ADD,
  ND_SUB,
  ND_MUL,
  ND_DIV,
};

typedef struct node_t node_t;
struct node_t {
  node_kind_t   kind;
  token_t*      tok;

  vector*   child;    // vector<node_t*>

  char*   name;
  size_t  len;
 
};


node_t*  node_new(node_kind_t kind);
node_t*  node_new_with_token(node_kind_t k, token_t* tok);
node_t*  node_new_with_lr(node_kind_t k, token_t* tok, node_t* lhs, node_t* rhs);
void node_free(node_t* node);

node_t**  node_append(node_t* node, node_t* item);

bool node_is_same_name(node_t* node, char const* name);

void print_node(node_t* nd);
