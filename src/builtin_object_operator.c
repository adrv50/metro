#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "utils.h"
#include "alert.h"
#include "vector.h"
#include "utf-convert.h"

#include "metro.h"
#include "builtin.h"

static inline int is_either_type(mt_type_kind K, mt_object* a,
                                 mt_object* b) {
  if (a->typeinfo.kind == K)
    return 1;

  if (b->typeinfo.kind == K)
    return 2;

  return 0;
}

static inline mt_object* to_float(mt_object* int_obj) {

  switch (int_obj->typeinfo.kind) {
  case TYPE_INT:
    int_obj->vf = (float)(int_obj->vi);
    break;

  default:
    todo_impl;
  }

  int_obj->typeinfo.kind = TYPE_FLOAT;

  return int_obj;
}

//
// -------------------------
//  ND_MUL
// -------------------------
//
mt_object* mul_object(mt_object* left, mt_object* right) {

  //
  // str * int
  if (IS_STRING(left) && IS_INT(right)) {
  _label_mul_str_int:
    mt_object* strobj = mt_obj_new_string();

    for (int i = 0; i < right->vi; i++)
      vector_append_vector(strobj->vs, left->vs);

    return strobj;
  }

  //
  // int * str ( --> swap and goto str*int )
  else if (IS_INT(left) && IS_STRING(right)) {
    swap(left, right);
    goto _label_mul_str_int;
  }

  else if (mt_obj_is_numeric(left) && mt_obj_is_numeric(right)) {
    if( IS_INT(left) && IS_INT(right) )
      return mt_obj_new_int(left->vi * right->vi);

    if( IS_FLOAT(left) && IS_FLOAT(right) )
      return mt_obj_new_float(left->vf * right->vf);

    if( IS_INT(left) )
      return mt_obj_new_float((double)left->vi * right->vf);

    return mt_obj_new_float(left->vf* (double)right->vi);
  }

  todo_impl;
  return left;
}

//
// -------------------------
//  ND_DIV
// -------------------------
//
mt_object* div_object(mt_object* left, mt_object* right) {

  switch (left->typeinfo.kind) {
  case TYPE_FLOAT:
    if (IS_FLOAT(right))
      left->vf /= right->vf;
    else if (IS_INT(right))
      left->vf /= (float)right->vi;
    else
      todo_impl;

    break;

  case TYPE_INT:
    if (IS_FLOAT(right))
      left->vi /= (int)right->vf;
    else if (IS_INT(right))
      left->vi /= right->vi;
    else
      todo_impl;

    break;

  default:
    todo_impl;
  }

  return left;
}

//
// -------------------------
//  ND_MOD
// -------------------------
//
mt_object* mod_object(mt_object* left, mt_object* right) {

  debug(assert(IS_INT(left) && IS_INT(right)));

  left->vi %= right->vi;

  return left;
}

//
// -------------------------
//  ND_ADD
// -------------------------
//
mt_object* add_object(mt_object* left, mt_object* right) {

  //
  // どっちも数値型
  if (mt_obj_is_numeric(left) && mt_obj_is_numeric(right)) {
    //
    // どちらかが Float であれば、両方 Float にする
    if (left->typeinfo.kind == TYPE_FLOAT) {
      if (right->typeinfo.kind != TYPE_FLOAT)
        right = to_float(right);
    }
    else if (right->typeinfo.kind == TYPE_FLOAT) {
      if (left->typeinfo.kind != TYPE_FLOAT)
        left = to_float(left);
    }

    assert(left->typeinfo.kind == right->typeinfo.kind);

    switch (left->typeinfo.kind) {
    case TYPE_INT:
      left->vi += right->vi;
      break;

    case TYPE_FLOAT:
      left->vf += right->vf;
      break;
    }
  }

  //
  // str + str
  else if (IS_STRING(left) && IS_STRING(right)) {
    vector_append_vector(left->vs, right->vs);
  }

  else {
    todo_impl;
  }

  return left;
}

//
// -------------------------
//  ND_SUB
// -------------------------
//
mt_object* sub_object(mt_object* left, mt_object* right) {

  //
  // どっちも数値型
  if (mt_obj_is_numeric(left) && mt_obj_is_numeric(right)) {
    //
    // どちらかが Float であれば、両方 Float にする
    if (left->typeinfo.kind == TYPE_FLOAT) {
      if (right->typeinfo.kind != TYPE_FLOAT)
        right = to_float(right);
    }
    else if (right->typeinfo.kind == TYPE_FLOAT) {
      if (left->typeinfo.kind != TYPE_FLOAT)
        left = to_float(left);
    }

    assert(left->typeinfo.kind == right->typeinfo.kind);

    switch (left->typeinfo.kind) {
    case TYPE_INT:
      left->vi -= right->vi;
      break;

    case TYPE_FLOAT:
      left->vf -= right->vf;
      break;
    }
  }

  else {
    todo_impl;
  }

  return left;
}
