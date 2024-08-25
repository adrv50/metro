#include <stdio.h>
#include <stdlib.h>
#include "object.h"
#include "utf-convert.h"

mt_type_info mt_type_info_new(mt_type_kind kind) {
  mt_type_info ti = {0};

  ti.kind = kind;
  // ti.params = vector_new(sizeof(mt_type_info_t));

  return ti;
}

mt_object* mt_obj_new(mt_type_info typeinfo) {
  mt_object* obj = calloc(1, sizeof(mt_object));

  obj->typeinfo = typeinfo;

  return obj;
}

mt_object* mt_obj_new_int(i64 v) {
  mt_object* obj = mt_obj_new(mt_type_info_new(TYPE_INT));

  obj->vi = v;

  return obj;
}

mt_object* mt_obj_new_float(double v) {
  mt_object* obj = mt_obj_new(mt_type_info_new(TYPE_FLOAT));

  obj->vf = v;

  return obj;
}

mt_object* mt_obj_new_bool(bool v) {
  mt_object* obj = mt_obj_new(mt_type_info_new(TYPE_BOOL));

  obj->vb = v;

  return obj;
}

mt_object* mt_obj_new_char(u16 v) {
  mt_object* obj = mt_obj_new(mt_type_info_new(TYPE_CHAR));

  obj->vc = v;

  return obj;
}

mt_object* mt_obj_new_string() {
  mt_object* obj = mt_obj_new(mt_type_info_new(TYPE_STRING));

  obj->vs = vector_new(sizeof(u16));

  return obj;
}

mt_object* mt_obj_new_vector() {
  mt_object* obj = mt_obj_new(mt_type_info_new(TYPE_VECTOR));

  obj->vv = vector_new(sizeof(mt_object*));

  return obj;
}

bool mt_obj_is_numeric(mt_object* obj) {
  switch (obj->typeinfo.kind) {
  case TYPE_INT:
  case TYPE_FLOAT:
    return true;
  }

  return false;
}

void print_object(mt_object* obj) {
  switch (obj->typeinfo.kind) {
  case TYPE_INT:
    printf("%lu", obj->vi);
    break;

  case TYPE_FLOAT:
    printf("%lf", obj->vf);
    break;

  case TYPE_BOOL:
    printf(obj->vb ? "true" : "false");
    break;

  case TYPE_CHAR: {
    char buf[10] = {0};
    utf16_to_utf8((u8*)buf, &obj->vc, 3);
    printf("%s", buf);
    break;
  }

  case TYPE_STRING: {
    size_t const buflen = obj->vs->count * 3;
    char buf[buflen];

    size_t res = utf16_to_utf8((u8*)buf, (u16*)obj->vs->_data, buflen);

    buf[res] = 0;

    printf("%s", buf);
    break;
  }

  case TYPE_VECTOR: {
    printf("[");

    for (size_t i = 0; i < obj->vv->count; i++) {
      print_object(vector_get_as(mt_object*, obj->vv, i));

      if (i < obj->vv->count - 1)
        printf(", ");
    }

    printf("]");
    break;
  }
  }
}