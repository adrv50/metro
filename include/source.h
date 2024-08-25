#pragma once

#include <stdlib.h>

typedef struct {
  char* path;
  char* data;
  size_t size; // size of actual data
  size_t length;
} source_file;

//
// create an instance of source,
// and read from the file 'path'
source_file* source_file_new(char* path);

void source_file_free(source_file* source);
