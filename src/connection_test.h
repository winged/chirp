// ======================
// Connection test Header
// ======================
//
// .. code-block:: cpp

#ifndef ch_connection_test_h
#define ch_connection_test_h

#include "connection.h"

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

#endif //ch_connection_test_h
