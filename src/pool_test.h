// ================
// Pool test Header
// ================
//
// .. todo:: Document purpose
//
// .. code-block:: cpp

#ifndef ch_pool_test_h
#define ch_pool_test_h

// Project includes
// ================
//
// .. code-block:: cpp
//
#include "buffer.h"

// .. c:function::
extern
void
test_ch_bf_init(ch_buffer_pool_t* pool, uint8_t max_buffers);

// .. c:function::
extern
void
test_ch_bf_free(ch_buffer_pool_t* pool);

// .. c:function::
extern
ch_bf_handler_t*
test_ch_bf_reserve(ch_buffer_pool_t* pool);

// .. c:function::
extern
void
test_ch_bf_return(ch_buffer_pool_t* pool, ch_bf_handler_t* handler_buf);

#endif //ch_pool_test_h
