#pragma once

#include "node.h"
#include "object.h"

void mt_eval_init(void);
void mt_eval_exit(void);

mt_object* mt_eval_evalfull(node_t* node);
