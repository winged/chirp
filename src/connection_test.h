// ======================
// Connection test header
// ======================
//
// .. todo:: Document purpose
//
// .. code-block:: cpp

#ifndef ch_connection_test_h
#define ch_connection_test_h

// Project includes
// ================
//
// .. code-block:: cpp
//
#include "connection.h"

// .. c:function::
extern
void
test_ch_cn_conn_dict(
        ch_connection_t* x,
        ch_connection_t* y,
        int* cmp,
        int* was_inserted,
        int* len,
        int* x_mem,
        int* y_mem
);
//    :noindex:

#endif //ch_connection_test_h
