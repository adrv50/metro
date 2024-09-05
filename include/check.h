#pragma once

#include "node.h"

struct mt_ck_checked_log_t {

  union {
    //
    // identifier or scope-resol
    struct {
      bool is_found;
      mt_node* ptr_to;

    } when_ident;
  };
};

typedef struct mt_ck_checked_log_t mt_ck_checked_log_t;

//
// evaluate type of expr
//
mt_type_info mt_ck_type_eval(mt_node* node);

//
// check node
//
void mt_ck_check(mt_node* node);
