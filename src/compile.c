#include <string.h>
#include <stdarg.h>
#include <assert.h>

#include "alert.h"
#include "compile.h"

static vector*  datalist;

static void emit(char const* fmt, ...) {
  static char buf[0x100];

  va_list ap;
  
  va_start(ap, fmt);
  vsprintf(buf, fmt, ap);
  va_end(ap);

  printf("%s\n", buf);
}

static void compile(node_t* node) {
  static int reg = 0;
  static int label_num = 0;

  int label = label_num;

  if( !node )
    return;

  switch( node->kind ) {
    case ND_VALUE: {
      switch( node->tok->kind ) {
        case TOK_INT:
          emit("mov r%d, #%d", reg, node->tok->val);
          break;
      }

      return;
    }

    case ND_IF: {
      label_num++;

      compile(nd_if_cond(node));

      emit("cmp r%d, #0", reg);
      emit("beq _label_%d", label);

      compile(nd_if_true(node));

      emit("_label_%d:", label);

      return;
    }

    case ND_BLOCK: {
      for( size_t i = 0; i < node->child->count; i++ )
        compile(nd_get_child(node, i));

      return;
    }
  }

  compile(nd_lhs(node));

  reg++;
  compile(nd_rhs(node));

  reg--;

  switch( node->kind ) {
    case ND_ADD:
      emit("add r%d, r%d", reg, reg + 1);
      break;

    case ND_SUB:
      emit("sub r%d, r%d", reg, reg + 1);
      break;

    case ND_MUL:
      emit("mul r%d, r%d", reg, reg + 1);
      break;

    case ND_DIV:
      emit("div r%d, r%d", reg, reg + 1);
      break;

    default:
      alertfmt("%d", node->kind);
      todo_impl;
  }
}

asm_operand_t make_asm_op(asm_op_kind_t kind, int rdest, int rsrc) {
  asm_operand_t op = { 0 };

  op.kind   = kind;
  op.rdest  = rdest;
  op.rsrc   = rsrc;

  return op;
}

void compiler_compile_full(node_t* node) {
  assert(node->kind == ND_PROGRAM);

  for( size_t i = 0; i < node->child->count; i++ )
    compile(nd_get_child(node, i));
}