#include <string.h>
#include "metro.h"

static mt_error  err_top;

mt_error*  mt_new_error(mt_err_kind_t kind, char const* msg, token_t* token) {

}

mt_error*  mt_new_error_from_node(mt_err_kind_t kind, char const* msg, node_t* node) {

}

void  mterr_emit(mt_error* err) {

}

mtdriver* driver_new(char* path) {

}

void driver_free(mtdriver* dr) {

}

source_t* driver_get_current_source(mtdriver* dr) {

}

void  metro_init() {

}

void  metro_exit() {
  
}
