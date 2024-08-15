#pragma once

#include "alert.h"
#include "types.h"
#include "lexer.h"
#include "parser.h"
#include "node.h"

typedef uint16_t  mt_err_kind_t;

enum {
  // unknown character on source code
  ERR_INVALID_TOKEN,

  // parser: invalid syntax
  ERR_INVALID_SYNTAX,

  // parser: unexpected token
  ERR_UNEXPECTED_TOKEN,

};

typedef struct mt_error mt_error;
struct mt_error {
  mt_err_kind_t     kind;
  char const*       msg;

  size_t            position;
  token_t*          token;
  node_t*           node;

  mt_error*         _next;
};

mt_error*
mt_new_error(mt_err_kind_t kind, char const* msg, size_t pos);

mt_error*
mt_new_error_from_token(mt_err_kind_t kind, char const* msg, token_t* token);

mt_error*
mt_new_error_from_node(mt_err_kind_t kind, char const* msg, node_t* node);

void  mt_error_emit(mt_error* err);

// emit error and exit.
void  mt_abort_with(mt_error* err);

typedef struct mtdriver mtdriver;
struct mtdriver {
  mtlexer*  lexer;
};

// create a new driver instance
mtdriver* driver_new(char* path);

// delete memory
void driver_free(mtdriver* dr);

// get the current compiling source
source_t* driver_get_current_source(mtdriver* dr);


void  metro_init();
void  metro_exit();
