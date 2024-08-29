#pragma once

#include "alert.h"
#include "types.h"
#include "node.h"
#include "lexer.h"

typedef enum {
  // unknown character on source code
  ERR_INVALID_TOKEN,

  // parser: invalid syntax
  ERR_INVALID_SYNTAX,

  ERR_TYPE_MISMATCH,

  // parser: unexpected token
  ERR_UNEXPECTED_TOKEN,

  // parser: brackets is not closed
  ERR_NOT_CLOSED_BRACKETS,

} mt_err_kind_t;

typedef struct mt_error_tag {
  mt_err_kind_t kind;
  char const* msg;

  size_t position;
  mt_token* token;
  mt_node* node;

  struct mt_error_tag* _next;
} mt_error;

mt_error* mt_new_error(mt_err_kind_t kind, char const* msg,
                       size_t pos);

mt_error* mt_new_error_from_token(mt_err_kind_t kind, char const* msg,
                                  mt_token* token);

mt_error* mt_new_error_from_node(mt_err_kind_t kind, char const* msg,
                                 mt_node* node);

void mt_error_emit(mt_error* err);

// emit error and exit.
void mt_abort_with(mt_error* err) __attribute__((__noreturn__));

typedef struct mtdriver mtdriver;
struct mtdriver {
  source_file* source;
  mtlexer* lexer;
};

// create a new driver instance
mtdriver* driver_new(char* path);

// delete memory
void driver_free(mtdriver* dr);

int driver_main(mtdriver* dr, int argc, char** argv);

// get the current compiling source
source_file* driver_get_current_source(mtdriver* dr);

void metro_init();
void metro_exit();
