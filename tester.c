#include "include/metro.h"

int main() {
  mt_object* obj = mt_obj_new_int(123);

  printf("%ld\n", obj->vi);
}