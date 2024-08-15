#include <stdio.h>

#include "alert.h"
#include "vector.h"

#include "lexer.h"
#include "parser.h"

#include "metro.h"

int main(int argc, char** argv) {
  return driver_main(driver_new("test.metro"), argc, argv);
}