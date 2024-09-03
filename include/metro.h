#pragma once

#include "alert.h"
#include "types.h"
#include "node.h"
#include "lexer.h"

typedef struct {
  source_file* source;
  mtlexer* lexer;
} mtdriver;

// create a new driver instance
mtdriver* mt_driver_new(char* path);

// delete memory
void mt_driver_free(mtdriver* dr);

int mt_driver_main(mtdriver* dr, int argc, char** argv);

// get the current compiling source
source_file* mt_driver_get_current_source(mtdriver* dr);

void metro_init();
void metro_exit();
