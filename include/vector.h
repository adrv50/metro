#pragma once

#include <stddef.h>
#include "types.h"

#define vector_get(_V, _N) ((_V)->_data + (_V)->type_width * (_N))
#define vector_get_as(_T, _V, _N) (((_T*)((_V)->_data))[_N])

#define vector_begin(_V) ((_V)->_data)
#define vector_end(_V) ((_V)->_data + (_V)->type_width * (_V)->count)
#define vector_last(_V) (vector_get((_V), (_V)->count - 1))

#define vector_size(_V) ((_V)->type_width * (_V)->count)

typedef struct vector_tag {
  u16 type_width; // size of the type of element
  size_t count;   // elements count

  size_t _actual_count;
  void* _data;
  size_t _datasize;
} vector;

// constructors
vector* vector_new(u16 type_width);
vector* vector_new_with_count(u16 type_width, size_t count);
vector* vector_copy(vector* v);

// destructor
void vector_free(vector* v);

vector* vector_resize(vector* v, size_t n);
vector* vector_extend(vector* v, size_t add_count);

// add item
void* vector_append(vector* v, void* item);
void* vector_append_vector(vector* v, vector* vec);

// insert
void* vector_insert(vector* v, size_t index, void* item);
void* vector_insert_vector(vector* v, size_t index, vector* vec);

void vector_clear(vector* v);

void vector_pop_back(vector* v);

void vector_erase(vector* v, size_t index);
void vector_erase_range(vector* v, size_t begin, size_t end);
