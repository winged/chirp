// ===============
// Connection test
// ===============
//
// .. todo:: Document purpose
//
// .. code-block:: cpp

// Project includes
// ================
//
// .. code-block:: cpp
//
#include "connection_test.h"

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
)
//    :noindex:
//
// .. code-block:: cpp
//
{
    ch_connection_t* dict = NULL;
    ch_connection_t* m = NULL;

    *cmp = ch_connection_cmp(x, y);
    sglib_ch_connection_t_add_if_not_member(&dict, x, &m);
    *was_inserted = sglib_ch_connection_t_add_if_not_member(&dict, y, &m);
    *len = sglib_ch_connection_t_len(dict);
    *x_mem = sglib_ch_connection_t_is_member(dict, x);
    *y_mem = sglib_ch_connection_t_is_member(dict, y);
}
