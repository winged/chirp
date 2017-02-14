// =========
// Pool test
// =========
//
// .. code-block:: cpp
//
#include "pool_test.h"

void
test_ch_bf_init(ch_buffer_pool_t* pool, uint8_t max_buffers)
{
    ch_bf_init(pool, max_buffers);
}

void
test_ch_bf_free(ch_buffer_pool_t* pool)
{
    ch_bf_free(pool);
}

ch_bf_handler_t*
test_ch_bf_reserve(ch_buffer_pool_t* pool)
{
    return ch_bf_reserve(pool);
}

void
test_ch_bf_return(ch_buffer_pool_t* pool, ch_bf_handler_t* handler_buf)
{
    ch_bf_return(pool, handler_buf);
}
