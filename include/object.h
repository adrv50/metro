#pragma once

#include "typeinfo.h"
#include "vector.h"

//
//  mt_object:
//    the data structure of value in metro.
//
//  rules:
//    1. do not change type-info dinamicly.
//
typedef struct {
  // type-info of object
  mt_type_info typeinfo;

  union {
    i64 vi;
    double vf;
    bool vb;
    u16 vc;

    // when TYPE_STRING
    vector* vs; // => vector<u16>

    // when TYPE_VECTOR
    vector* vv; // => vector<mt_object*>
  };
} mt_object;

mt_object* mt_obj_new(mt_type_info typeinfo);

mt_object* mt_obj_new_int(i64 v);
mt_object* mt_obj_new_float(double v);
mt_object* mt_obj_new_bool(bool v);
mt_object* mt_obj_new_char(u16 v);
mt_object* mt_obj_new_string();
mt_object* mt_obj_new_vector();

bool mt_obj_is_numeric(mt_object* obj);

void print_object(mt_object* obj);