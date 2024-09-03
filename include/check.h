#pragma once

#include "node.h"

//
// evaluate type of expr
//
mt_type_info mt_ck_type_eval(mt_node* node);

//
// check node
//
void mt_ck_check(mt_node* node);
