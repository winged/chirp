// =========
// Pool test
// =========
//
// .. todo:: Document purpose
//
// .. code-block:: cpp

// Project includes
// ================
//
// .. code-block:: cpp
//
#include "pool_test.h"

// .. c:function::
void
test_ch_bf_init(ch_buffer_pool_t* pool, uint8_t max_buffers)
//    :noindex:
//
// .. code-block:: cpp
//
{
    ch_bf_init(pool, max_buffers);
}

// .. c:function::
void
test_ch_bf_free(ch_buffer_pool_t* pool)
//    :noindex:
//
// .. code-block:: cpp
//
{
    ch_bf_free(pool);
}

// .. c:function::
ch_bf_handler_t*
test_ch_bf_reserve(ch_buffer_pool_t* pool)
//    :noindex:
//
// .. code-block:: cpp
//
{
    return ch_bf_reserve(pool);
}

// .. c:function::
void
test_ch_bf_return(ch_buffer_pool_t* pool, ch_bf_handler_t* handler_buf)
//    :noindex:
//
// .. code-block:: cpp
//
{
    ch_bf_return(pool, handler_buf);
}
