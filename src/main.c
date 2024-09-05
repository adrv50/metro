#include "metro.h"

int main(int argc, char** argv) {
  metro_init();
  return mt_driver_main(mt_driver_new("test.metro"), argc, argv);
}