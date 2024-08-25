#include <stdio.h>
#include <string.h>

#include "alert.h"
#include "node.h"

mt_node* node_new(mt_node_kind kind) {
  return node_new_with_lr(kind, NULL, NULL, NULL);
}

mt_node* node_new_with_token(mt_node_kind kind, mt_token* tok) {
  return node_new_with_lr(kind, tok, NULL, NULL);
}

mt_node* node_new_with_lr(mt_node_kind kind, mt_token* tok,
                          mt_node* lhs, mt_node* rhs) {
  mt_node* nd = calloc(1, sizeof(mt_node));

  nd->kind = kind;
  nd->tok = tok;
  nd->child = vector_new(sizeof(mt_node*));

  if (lhs)
    node_append(nd, lhs);

  if (rhs)
    node_append(nd, rhs);

  return nd;
}

void node_free(mt_node* node) {
  if (!node)
    return;

  for (size_t i = 0; i < node->child->count; i++) {
    node_free(*(mt_node**)vector_get(node->child, i));
  }

  vector_free(node->child);
  free(node);
}

mt_node** node_append(mt_node* node, mt_node* item) {
  return (mt_node**)vector_append(node->child, &item);
}

bool node_is_same_name(mt_node* node, char const* name) {
  size_t len = strlen(name);

  return node->len == len && strncmp(node->name, name, len) == 0;
}

void print_node(mt_node* node) {
  switch (node->kind) {
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

  case ND_PROGRAM:
    for (size_t i = 0; i < node->child->count; i++) {
      print_node(nd_get_child(node, i));
      puts("");
    }

    break;
  }
}