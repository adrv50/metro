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

} asm_op_kind;

typedef struct {
  asm_op_kind   kind;

  


} asm_operand;

