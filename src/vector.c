#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "vector.h"

#define  VEC_INITIALIZE_COUNT     50

static void* memcpyex(void* dest, void* src, size_t size) {
  if( dest == src )
    return dest;

  if( src < dest ) {
    for( size_t i = 0; i < size; i++ )
      ((char*)dest)[size - i - 1] = ((char*)src)[size - i - 1];

    return dest;
  }

  return memcpy(dest, src, size);
}

static void* vector_realloc(vector* v) {
  v->_actual_count  *= 2;
  v->_datasize      = v->type_width * v->_actual_count;

  return (v->_data = realloc(v->_data, v->_datasize));
}


// static void vector_check_buffer(vector* v) {
//   if( v->count >= v->_actual_count / 2 )
//     vector_realloc(v);
// }


// v に対して要素を n 個追加できるかどうか確認して、
// バッファが足りない場合は拡張する
static void vector_make_buffer(vector* v, size_t n) {
  if( v->count + n >= v->_actual_count / 2 )
    vector_realloc(v);
}

vector*  vector_new(u16 type_width) {
  vector* v = malloc(sizeof(vector));

  v->type_width     = type_width;
  v->count          = 0;

  v->_actual_count  = VEC_INITIALIZE_COUNT;

  v->_datasize      = v->type_width * v->_actual_count;
  v->_data          = calloc(1, v->_datasize);

  return v;
}

vector*  vector_new_with_count(u16 type_width, size_t count) {
  vector* v = vector_new(type_width);

  v->_actual_count = v->count = count;

  vector_realloc(v);

  return v;
}

vector*  vector_copy(vector* v) {
  vector* ret = malloc(sizeof(vector));

  ret->type_width     = v->type_width;
  ret->count          = v->count;
  ret->_actual_count  = v->_actual_count;
  ret->_data          = malloc(v->_datasize);
  ret->_datasize      = v->_datasize;

  memcpy(ret->_data, v->_data, ret->_datasize);

  return ret;
}

void  vector_free(vector* v) {
  free(v->_data);
  free(v);
}

void* vector_append(vector* v, void* item) {
  vector_make_buffer(v, 1);
  
  v->count++;
  return memcpy(vector_last(v), item, v->type_width);
}

void* vector_append_vector(vector* v, vector* vec) {
  assert(v->type_width == vec->type_width);
  
  vector_make_buffer(v, vec->count);

  v->count += vec->count;

  return
    memcpy(vector_get(v, v->count - vec->count), vec->_data, vec->_datasize);
}

void* vector_insert(vector* v, size_t index, void* item) {
  assert(index <= v->count);
  vector_make_buffer(v, 1);

  memcpyex(vector_get(v, index + 1),
      vector_get(v, index), v->type_width * (v->count - index));

  return memcpy(vector_get(v, index), item, v->type_width);
}

void* vector_insert_vector(vector* v, size_t index, vector* vec) {
  assert(v->type_width == vec->type_width);
  assert(index < v->count);
  vector_make_buffer(v, vec->count);

  memcpyex(vector_get(v, index + vec->count),
    vector_get(v, index), v->type_width * (v->count - index));

  v->count += vec->count;

  return memcpy(vector_get(v, index), vec->_data, vector_size(vec));
}

void vector_pop_back(vector* v) {
  assert(v->count > 0);

  v->count--;
}

void vector_erase(vector* v, size_t index) {
  assert(v->count > 0);
  assert(index < v->count);

  memcpy(vector_get(v, index),
    vector_get(v, index + 1), (v->count - 1) * v->type_width);

  v->count--;
}

void vector_erase_range(vector* v, size_t begin, size_t end) {
  assert(begin < end);
  assert(end < v->count);

  memcpyex(
    vector_get(v, begin), vector_get(v, end), v->type_width * (v->count - end));

  v->count -= (end - begin);
}