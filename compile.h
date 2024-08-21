#pragma once

#include "node.h"
#include "typeinfo.h"

typedef enum {
  ASM_MOV,
  ASM_CMP,

  ASM_ADD,
  ASM_SUB,

  ASM_LABEL,
  ASM_DATA,

} asm_op_kind_t;

typedef enum {
  ASM_DATA_U8,
  ASM_DATA_U16,  // or char16_t
  ASM_DATA_U32,
  ASM_DATA_U64,
  ASM_DATA_STRING,  // UTF-16

} asm_data_type_t;

typedef struct {
  asm_op_kind_t kind;

  int rdest;
  int rsrc;

} asm_operand;

asm_operand make_asm_op(asm_op_kind_t kind, int rdest, int rsrc);

void compiler_compile_full(node_t* node);
