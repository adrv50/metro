#pragma once

#include <stdlib.h>

typedef struct source_t source_t;
struct source_t {
  char* path;
  char* data;
  size_t size;  // size of actual data
  size_t length;
};

//
// create an instance of source,
// and read from the file 'path'
source_t* source_new(char* path);

void source_free(source_t* source);
