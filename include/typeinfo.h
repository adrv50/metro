#pragma once

#include "types.h"
#include "vector.h"

typedef enum {
  TYPE_NONE,
  TYPE_INT,
  TYPE_FLOAT,
  TYPE_BOOL,
  TYPE_CHAR,
  TYPE_STRING,
  TYPE_VECTOR,
} mt_type_kind;

typedef struct {
  mt_type_kind kind;
  vector* params; // vector<mt_type_info_t>
} mt_type_info_t;

mt_type_info_t mt_typeinfo_new(mt_type_kind kind);
