#pragma once

#include "types.h"
#include "vector.h"

enum {
  TYPE_NONE,
  TYPE_INT,
  TYPE_FLOAT,
  TYPE_BOOL,
  TYPE_CHAR,
  TYPE_STRING,
  TYPE_VECTOR,
};

typedef u8  type_kind;

typedef struct {
  type_kind   kind;
  vector*     params;
} mt_type_info_t;


mt_type_info_t  make_typeinfo(type_kind kind);
