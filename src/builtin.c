#include "metro.h"
#include "builtin.h"

// 引数 argc は、実際に呼ばれたときの引数の数が格納される

static mt_object* _bltin_print(int argc, mt_object** args) {
  for (int i = 0; i < argc; i++)
    print_object(args[i]);

  return mt_obj_new_int(0);
}

// clang-format off
static mt_builtin_func_t _builtin_functions[] = {
  { "print", 5, _bltin_print, (mt_type_info[]){{TYPE_STRING}}, -1, {TYPE_INT} }
};

static const int _bfunctions_count =
  sizeof(_builtin_functions) / sizeof(mt_builtin_func_t);
// clang-format on

mt_builtin_func_t const* mt_get_builtin_functions() {
  return _builtin_functions;
}

int mt_get_builtin_functions_count() {
  return _bfunctions_count;
}