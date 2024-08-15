#include "eval.h"

typedef struct {
  vector*     enums;          // node_t*
  vector*     structs;        // node_t*
  vector*     classes;        // node_t*
  vector*     functions;      // node_t*
  vector*     methods;        // node_t*
  vector*     namespaces;     // namespace_data
} namespace_data;

typedef struct {
  vector*             uses;   // vector<char*>
  namespace_data*     global;
} program;

typedef struct {
  namespace_data*     namespace;
  node_t*             node;
} name_find_result;

static program  g_program;

static namespace_data* ns_new(node_t* node) {
  namespace_data* ns = calloc(1, sizeof(namespace_data));

  node_t* begin = vector_begin(node->child);
  node_t* end = vector_end(node->child);

  for( ; begin != end; begin++ ) {
    
  }

  return ns;
}

static void ns_free(namespace_data* ns) {

}

static bool  ns_find_name(namespace_data* ns, name_find_result* res, char* name) {
  node_t* nd;

  vector* findlist[] = { ns->enums, ns->structs, ns->classes, ns->functions, ns->methods };
  size_t const findlist_size = sizeof(findlist) / sizeof(vector*);

  for( vector** vec = findlist; vec != findlist + findlist_size; vec++ ) {
    for( nd = vector_begin(*vec); nd != vector_end(*vec); nd++ ) {
      if( node_is_same_name(nd, name) == 0 ) {
        res->namespace = ns;
        res->node = nd;

        return true;
      }
    }
  }

  for( namespace_data* p = vector_begin(ns->namespaces); p != vector_end(ns->namespaces); p++ ) {
    if( ns_find_name(p, res, name) )
      return true;
  }

  return false;
}

void evaluator_init(node_t* program) {
  g_program.global = ns_new(program);
}

void evaluator_exit() {

}

mt_object* evaluator_evalnode(node_t* node) {

}

