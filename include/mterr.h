
#include "alert.h"
#include "types.h"
#include "node.h"
#include "lexer.h"

#include "node.h"

typedef enum {
  // unknown character on source code
  ERR_INVALID_TOKEN,

  // parser: invalid syntax
  ERR_INVALID_SYNTAX,

  ERR_TYPE_MISMATCH,

  ERR_UNDEFINED_VARIABLE,

  ERR_CANNOT_FIND_NAME,

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

void mt_err_init();
void mt_err_exit();

mt_error* mt_add_error(mt_err_kind_t kind, char const* msg,
                       size_t pos);

mt_error* mt_add_error_from_token(mt_err_kind_t kind, char const* msg,
                                  mt_token* token);

mt_error* mt_add_error_from_token_with_fmt(mt_err_kind_t kind,
                                           mt_token* token,
                                           char const* fmt, ...);

mt_error* mt_add_error_from_node(mt_err_kind_t kind, char const* msg,
                                 mt_node* node);

void mt_error_emit(mt_error* err);

// emit all error and exit.
void mt_error_emit_and_exit() __attribute__((__noreturn__));

void mt_error_check();
