#pragma once

#include "typeinfo.h"
#include "vector.h"
#include "builtin.h"

#define IS_NONE(obj) (obj->typeinfo.kind == TYPE_NONE)
#define IS_INT(obj) (obj->typeinfo.kind == TYPE_INT)
#define IS_FLOAT(obj) (obj->typeinfo.kind == TYPE_FLOAT)
#define IS_BOOL(obj) (obj->typeinfo.kind == TYPE_BOOL)
#define IS_CHAR(obj) (obj->typeinfo.kind == TYPE_CHAR)
#define IS_STRING(obj) (obj->typeinfo.kind == TYPE_STRING)
#define IS_VECTOR(obj) (obj->typeinfo.kind == TYPE_VECTOR)

struct mt_node;
typedef struct mt_node mt_node;

//
//  mt_object:
//    the data structure of value in metro.
//
//  rules:
//    1. do not change type-info dinamicly.
//
struct mt_object {
  // type-info of object
  mt_type_info typeinfo;

  union {
    i64 vi;
    double vf;
    bool vb;
    u16 vc;

    struct {
      struct mt_node* vfn;
      mt_builtin_func_t const* vfn_builtin;
    };

    // when TYPE_STRING
    vector* vs; // => vector<u16>

    // when TYPE_VECTOR
    vector* vv; // => vector<mt_object*>
  };
};

mt_object* mt_obj_new(mt_type_info typeinfo);

// when you want to force free object
void mt_obj_force_free(mt_object* obj);

mt_object* mt_obj_new_int(i64 v);
mt_object* mt_obj_new_float(double v);
mt_object* mt_obj_new_bool(bool v);
mt_object* mt_obj_new_char(u16 v);
mt_object* mt_obj_new_string(void);
mt_object* mt_obj_new_vector(void);

mt_object* mt_obj_new_func(mt_node* func);
mt_object* mt_obj_new_blt_func(mt_builtin_func_t const* builtin);

mt_object* mt_obj_clone(mt_object* obj);

bool mt_obj_is_numeric(mt_object* obj);

void print_object(mt_object* obj);