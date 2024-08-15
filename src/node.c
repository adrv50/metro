#include <stdio.h>
#include <string.h>
#include "node.h"

node_t*  node_new(node_kind_t kind) {
  return node_new_with_lr(kind, NULL, NULL, NULL);
}

node_t*  node_new_with_token(node_kind_t kind, token_t* tok) {
  return node_new_with_lr(kind, tok, NULL, NULL);
}

node_t*  node_new_with_lr(node_kind_t kind, token_t* tok, node_t* lhs, node_t* rhs) {
  node_t* nd = calloc(1, sizeof(node_t));

  nd->kind    = kind;
  nd->tok     = tok;
  nd->child   = vector_new(sizeof(node_t*));

  node_append(nd, &lhs);
  node_append(nd, &rhs);

  return nd;
}

void node_free(node_t* node) {
  for( size_t i = 0; i < node->child->count; i++ ) {
    node_free(*(node_t**)vector_get(node->child, i));
  }

  vector_free(node->child);
  free(node);
}

node_t** node_append(node_t* node, node_t** item) {
  return (node_t**)vector_append(node->child, item);
}

bool node_is_same_name(node_t* node, char const* name) {
  size_t len = strlen(name);

  return node->len >= len && strncmp(node->name, name, len) == 0;
}

void print_node(node_t* node) {
  switch( node->kind ) {
    case ND_VALUE:
    case ND_VARIABLE:
      printf("%.*s", (int)node->tok->len, node->tok->str);
      break;

    case ND_ADD:
    case ND_SUB:
    case ND_MUL:
    case ND_DIV:
      print_node(nd_lhs(node));
      printf(" %.*s ", (int)node->tok->len, node->tok->str);
      print_node(nd_rhs(node));
      break;
  }
}