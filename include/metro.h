#pragma once

#include "alert.h"
#include "types.h"
#include "node.h"
#include "lexer.h"

typedef struct {
  source_file* source;
  mtlexer* lexer;
} mt_driver;

// create a new driver instance
mt_driver* mt_driver_new(char* path);

// delete memory
void mt_driver_free(mt_driver* dr);

int mt_driver_main(mt_driver* dr, int argc, char** argv);

mt_driver* mt_driver_get_cur_instance();

// get the current compiling source
source_file* mt_driver_get_current_source(mt_driver* dr);

void metro_init();
void metro_exit();
