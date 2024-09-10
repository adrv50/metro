#pragma once

// #include "node.h"

typedef mt_object* (*builtin_func_ptr_t)(int, mt_object**);

typedef struct mt_builtin_func_t mt_builtin_func_t;

struct mt_builtin_func_t {
  char const* name;
  int namelen;

  builtin_func_ptr_t impl;

  mt_type_info* arg_types;
  int arg_count; // -1 is free args

  mt_type_info return_type;
};

mt_builtin_func_t const* mt_get_builtin_functions();
int mt_get_builtin_functions_count();