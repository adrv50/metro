#include <stdio.h>
#include "source.h"

source_file* source_file_new(char* path) {
  FILE* fp = fopen(path, "r");

  if (fp == NULL) {
    fprintf(stderr, "cannot open '%s'", path);
    return NULL;
  }

  source_file* s = calloc(1, sizeof(source_file));
  s->path = path;

  fseek(fp, 0, SEEK_END);

  s->length = s->size = ftell(fp);

  s->data = malloc(s->size + 2);

  fseek(fp, 0, SEEK_SET);

  if (fread(s->data, s->size, 1, fp) != 1)
    abort();

  if (s->size == 0 || s->data[s->size - 1] != '\n')
    s->data[s->size++] = '\n';

  s->data[s->size] = 0;

  fclose(fp);

  return s;
}

void source_file_free(source_file* s) {
  free(s->data);
  free(s);
}
