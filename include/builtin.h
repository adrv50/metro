#pragma once

#include "node.h"

typedef struct {
  char const* name;
  int namelen;

  mt_node* impl; // => ND_FUNCTION

  mt_type_info* arg_types;
  u16 arg_count;

  mt_type_info return_type;
} mt_builtin_func;
