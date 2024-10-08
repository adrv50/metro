#pragma once

#include "node.h"

typedef struct {
  source_file* src;
  mt_token* list;
  mt_token* endtok;
  mt_token* cur;
  mt_token* ate;

} parser_ctx;

mt_node* parser_parse(source_file* src, mt_token* list);
