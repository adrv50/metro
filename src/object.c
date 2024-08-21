#include <stdlib.h>
#include "object.h"

mt_typeinfo mt_typeinfo_new(mt_type_kind kind) {
  mt_typeinfo ti = {0};

  ti.kind = kind;
  ti.params = vector_new(sizeof(mt_typeinfo));

  return ti;
}

mt_object* mt_obj_new(mt_typeinfo typeinfo) {
  mt_object* obj = calloc(1, sizeof(mt_object));

  obj->typeinfo = typeinfo;

  return obj;
}

mt_object* mt_obj_new_int(i64 v) {
  mt_object* obj = mt_obj_new(mt_typeinfo_new(TYPE_INT));

  obj->vi = v;

  return obj;
}

mt_object* mt_obj_new_float(double v) {
  mt_object* obj = mt_obj_new(mt_typeinfo_new(TYPE_INT));

  obj->vf = v;

  return obj;
}

mt_object* mt_obj_new_bool(bool v) {
  mt_object* obj = mt_obj_new(mt_typeinfo_new(TYPE_INT));

  obj->vb = v;

  return obj;
}

mt_object* mt_obj_new_char(u16 v) {
  mt_object* obj = mt_obj_new(mt_typeinfo_new(TYPE_INT));

  obj->vc = v;

  return obj;
}

mt_object* mt_obj_new_string(u16* v) {
  mt_object* obj = mt_obj_new(mt_typeinfo_new(TYPE_INT));

  obj->vs = v;

  return obj;
}

mt_object* mt_obj_new_vector() {
  mt_object* obj = mt_obj_new(mt_typeinfo_new(TYPE_VECTOR));

  obj->vv = vector_new(sizeof(mt_object*));

  return obj;
}