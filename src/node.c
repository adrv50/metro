#include <stdio.h>
#include <string.h>
#include <stdarg.h>

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

static void vecprintf(vector* v, char const* fmt, ...) {

  // v = vector<char>

  static char buf[1024] = {0};

  va_list ap;
  va_start(ap, fmt);
  vsprintf(buf, fmt, ap);
  va_end(ap);

  int len = strlen(buf);

  char* pbuf = vector_end(v);

  vector_extend(v, len);
  memcpy(pbuf, buf, len);
}

static void node2s(vector* v /*vector<char> */, mt_node* node) {

  static int indent = 0;

  if (!node) {
    vecprintf(v, "(null)");
  }

  if (node->kind > _NDKIND_BEGIN_OF_LR_OP_EXPR_ &&
      node->kind < _NDKIND_END_OF_LR_OP_EXPR_) {
    node2s(v, nd_lhs(node));
    vecprintf(v, " %.*s ", (int)node->tok->len, node->tok->str);
    node2s(v, nd_rhs(node));
  }

  switch (node->kind) {
  case ND_VALUE:
    vecprintf(v, "%.*s", node->tok->len, node->tok->str);
    break;

  case ND_IDENTIFIER:
    vecprintf(v, "%.*s", node->len, node->name);

    if (node->child->count > 0) {
      vecprintf(v, "<");

      for (int i = 0; i < node->child->count; i++) {
        node2s(v, nd_get_child(node, i));
        if (i < node->child->count - 1)
          vecprintf(v, ", ");
      }

      vecprintf(v, ">");
    }

    break;

  case ND_SCOPE_RESOLUTION:
    node2s(v, nd_lhs(node));
    vecprintf(v, "::");
    node2s(v, nd_rhs(node));
    break;

  case ND_BLOCK: {
    indent++;

    vecprintf(v, "{");

    for (int i = 0; i < node->child->count; i++) {
    }

    vecprintf(v, "}");

    indent--;
    break;
  }
  }
}

char* node_to_string(mt_node* node) {
  vector* v = vector_new(sizeof(char));

  node2s(v, node);

  char* buf = v->_data;
  free(v);

  return buf;
}