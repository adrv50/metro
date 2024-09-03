#include "metro.h"

int main(int argc, char** argv) {
  return mt_driver_main(mt_driver_new("test.metro"), argc, argv);
}