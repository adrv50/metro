#pragma once

#include "types.h"
#include "vector.h"

typedef u8 mt_type_kind;

enum {
  TYPE_NONE,
  TYPE_INT,
  TYPE_FLOAT,
  TYPE_BOOL,
  TYPE_CHAR,
  TYPE_STRING,
  TYPE_VECTOR,
  TYPE_FUNCTION,
  TYPE_MODULE,
};

typedef struct {
  mt_type_kind kind;
  bool is_const;
  vector* params; // vector<mt_type_info_t>
} mt_type_info;

mt_type_info mt_type_info_new(mt_type_kind kind);

bool mt_type_is_equal(mt_type_info a, mt_type_info b);