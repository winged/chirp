// =============
// Buffer header
// =============
//
// Implements a buffer pool. There is header, actor and data buffer per chrip
// handler.
//
// .. code-block:: cpp
//
#ifndef ch_buffer_h
#define ch_buffer_h

// Project includes
// ================
//
// .. code-block:: cpp
//
#include "util.h"
#include "config.h"

// Declarations
// ============

// .. c:type:: ch_bf_handler_t
//
//    Preallocated buffer for a chirp handler.
//
//    .. c:member:: ch_buf* header
//
//       Preallocated buffer for the chirp header.
//
//    .. c:member:: char* actor
//
//       Preallocated buffer for the actor.
//
//    .. c:member:: ch_buf* data
//
//       Preallocated buffer for the data.
//
//    .. c:member:: uint8_t id
//
//       Identifier of the buffer.
//
//    .. c:member:: uint8_t used
//
//       Indicates how many times the buffer is/was used.
//
// .. code-block:: cpp
//
typedef struct ch_bf_handler_s {
    ch_buf* header[CH_BF_PREALLOC_HEADER];
    char*   actor[CH_BF_PREALLOC_ACTOR];
    ch_buf* data[CH_BF_PREALLOC_DATA];
    uint8_t id;
    uint8_t used;
} ch_bf_handler_t;

// .. c:type:: ch_buffer_pool_t
//
//    Contains the preallocated buffers for the chirp handlers.
//
//    .. c:member:: uint8_t max_buffers
//
//       Defines the maximum number of buffers.
//
//    .. c:member:: uint8_t used_buffers
//
//       Defines how many buffers are currently used.
//
//    .. c:member:: uint32_t free_buffers
//
//       Defeines how many buffers are currently free (and therefore may be
//       used).
//
//    .. c:member:: ch_bf_handler_t* handlers
//
//       Pointer of type ch_bf_handler_t to the actual handlers. See
//       :c:type:`ch_bf_handler_t`.
//
// .. code-block:: cpp
//
typedef struct ch_buffer_pool_s {
    uint8_t  max_buffers;
    uint8_t  used_buffers;
    uint32_t free_buffers;
    ch_bf_handler_t* handlers;
} ch_buffer_pool_t;

// Definitions
// ===========

// .. c:function::
static
ch_inline
void
ch_bf_free(ch_buffer_pool_t* pool)
//
//    Free the given buffer structure.
//
//    :param ch_buffer_pool_t* pool: The buffer pool structure to free
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
//    Initialize the given buffer pool structure using given max. buffers.
//
//    :param ch_buffer_pool_t* pool: The buffer pool object
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
ch_bf_acquire(ch_buffer_pool_t* pool)
//
//    Acquire and return a new handler buffer from the pool. If no handler can
//    be reserved NULL is returned.
//
//    :param ch_buffer_pool_t* pool: The buffer pool structure which the
//                                   reservation shall be made from.
//
//   :return: a pointer to a reserved handler buffer from the given buffer
//            pool. See :c:type:`ch_bf_handler_t`
//   :rtype:  ch_bf_handler_t
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
ch_bf_release(ch_buffer_pool_t* pool, ch_bf_handler_t* handler_buf)
//
//    Set given handler buffer as unused in the buffer pool structure and
//    (re-)add it to the list of free buffers.
//
//    .. todo:: Maybe use another name for this method as it does not seem to
//              return something?
//
//    :param ch_buffer_pool_t* pool: The buffer pool structure containing the
//                                   handler buffer
//    :param ch_bf_handler_t* handler_buf: The handler buffer which shall be
//                                         marked as free
//
// .. code-block:: cpp
//
{
    // .. todo:: Should we not assert first, that the given handler buffer
    //           actually IS not in the pool as free buffer?
    A(handler_buf->used == 1, "Double return of buffer.");
    handler_buf->used = 0;
    A(handler_buf->used == 0, "Buffer pool inconsistent.");
    pool->used_buffers -= 1;
    // Return the buffer
    pool->free_buffers |= (1 << (31 - handler_buf->id));
}

#endif //ch_buffer_h
