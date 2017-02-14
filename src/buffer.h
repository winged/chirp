// =============
// Buffer Header
// =============
//
// Implements a buffer pool. There is header, actor and data buffer per chrip
// handler.
//
// .. code-block:: cpp

#ifndef ch_buffer_h
#define ch_buffer_h

#include "util.h"

// .. c:type:: ch_bf_handler_t
//
//    Preallocated buffer for a chirp handler
//
//    .. c:member:: void* header
//
//    Preallocated buffer for the chirp header
//
//    .. c:member:: char* actor
//
//    Preallocated buffer for the actor
//
//    .. c:member:: void* data
//
//    Preallocated buffer for the data
//
// .. code-block:: cpp

typedef struct ch_bf_handler_s {
    ch_buf* header[CH_BF_PREALLOC_HEADER];
    char*   actor[CH_BF_PREALLOC_ACTOR];
    ch_buf* data[CH_BF_PREALLOC_DATA];
    uint8_t id;
    uint8_t used;
} ch_bf_handler_t;

// .. c:type:: ch_buffer_pool_t
//
//    Contains the preallocated buffers for the chirp handlers
//
//    .. c:member:: unsinged char state
//
// .. code-block:: cpp

typedef struct ch_buffer_pool_s {
    uint8_t  max_buffers;
    uint8_t  used_buffers;
    uint32_t free_buffers;
    ch_bf_handler_t* handlers;
} ch_buffer_pool_t;

// .. c:function::
static
ch_inline
void
ch_bf_free(ch_buffer_pool_t* pool)
//
//    Free the buffer structure
//
//    :param ch_buffer_pool_t* pool: The buffer pool structure
//    :param max_buffers: Buffers to allocate
//
// .. code-block:: cpp
//
{
    ch_free(pool->handlers);
}

// .. c:function::
static
ch_inline
void
ch_bf_init(ch_buffer_pool_t* pool, uint8_t max_buffers)
//
//    Initialize the buffer pool structure
//
//    :param ch_buffer_pool_t* pool: The buffer object
//    :param max_buffers: Buffers to allocate
//
// .. code-block:: cpp
//
{
    int i;
    A(max_buffers <= 32, "buffer.c can't handle more than 32 handlers");
    pool->used_buffers = 0;
    pool->max_buffers  = max_buffers;
    pool->handlers     = ch_alloc(max_buffers * sizeof(ch_bf_handler_t));
    pool->free_buffers = 0xFFFFFFFFU;
    pool->free_buffers <<= (32 - max_buffers);
    for(i = 0; i < max_buffers; ++i) {
        pool->handlers[i].id   = i;
        pool->handlers[i].used = 0;
    }
}

// .. c:function::
static
ch_inline
ch_bf_handler_t*
ch_bf_reserve(ch_buffer_pool_t* pool)
//
//    Reserve and get a handler buffer from the pool
//
//    :param ch_buffer_pool_t* pool: The buffer pool structure
//
// .. code-block:: cpp
//
{
    int free;
    ch_bf_handler_t* handler_buf;
    if(pool->used_buffers < pool->max_buffers) {
        pool->used_buffers += 1;
        free = ch_msb32(pool->free_buffers);
        // Reserve the buffer
        pool->free_buffers &= ~(1 << (free - 1));
        // The msb represents the first buffer. So the value is inverted.
        handler_buf = &pool->handlers[32 - free];
        A(handler_buf->used == 0, "Handler buffer already used.");
        handler_buf->used = 1;
        return handler_buf;
    }
    return NULL;
}

// .. c:function::
static
ch_inline
void
ch_bf_return(ch_buffer_pool_t* pool, ch_bf_handler_t* handler_buf)
//
//    Reserve and get a handler buffer from the pool
//
//    :param ch_buffer_pool_t* pool: The buffer pool structure
//    :param ch_bf_handler_t* handler_buf: The buffer pool structure
//
// .. code-block:: cpp
//
{
    A(handler_buf->used == 1, "Double return of buffer.");
    handler_buf->used = 0;
    A(handler_buf->used == 0, "Buffer pool inconsistent.");
    pool->used_buffers -= 1;
    // Return the buffer
    pool->free_buffers |= (1 << (31 - handler_buf->id));
}

#endif //ch_buffer_h
