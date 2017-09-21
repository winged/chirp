// =============
// Buffer header
// =============
//
// Implements a buffer pool. There is header and data buffer per chrip
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
#include "message.h"
#include "util.h"
#include "config.h"

// Declarations
// ============

// .. c:type:: ch_bf_handler_t
//
//    Preallocated buffer for a chirp handler.
//
//    .. c:member:: ch_message_t
//
//       Preallocated message.
//
//    .. c:member:: ch_buf* header
//
//       Preallocated buffer for the chirp header.
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
    ch_message_t msg;
    ch_buf  header[CH_BF_PREALLOC_HEADER];
    ch_buf  data[CH_BF_PREALLOC_DATA];
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
ch_error_t
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
    memset(pool, 0, sizeof(ch_buffer_pool_t));
    size_t pool_mem = max_buffers * sizeof(ch_bf_handler_t);
    pool->used_buffers = 0;
    pool->max_buffers  = max_buffers;
    pool->handlers     = ch_alloc(pool_mem);
    memset(pool->handlers, 0, pool_mem);
    if(!pool->handlers) {
        fprintf(
            stderr,
            "%s:%d Fatal: Could not allocate memory fo r buffers. "
            "ch_buffer_pool_t:%p\n",
            __FILE__,
            __LINE__,
            (void*) pool
        );
        return CH_ENOMEM;
    }
    pool->free_buffers = 0xFFFFFFFFU;
    pool->free_buffers <<= (32 - max_buffers);
    for(i = 0; i < max_buffers; ++i) {
        pool->handlers[i].id   = i;
        pool->handlers[i].used = 0;
    }
    return CH_SUCCESS;
}

// .. c:function::
static
ch_inline
ch_bf_handler_t*
ch_bf_acquire(ch_buffer_pool_t* pool, int* last)
//
//    Acquire and return a new handler buffer from the pool. If no handler can
//    be reserved NULL is returned.
//
//    :param ch_buffer_pool_t* pool: The buffer pool structure which the
//                                   reservation shall be made from.
//    :param int* last: Out param: This is the last handler buffer.
//
//   :return: a pointer to a reserved handler buffer from the given buffer
//            pool. See :c:type:`ch_bf_handler_t`
//   :rtype:  ch_bf_handler_t
//
// .. code-block:: cpp
//
{
    *last = 0;
    ch_bf_handler_t* handler_buf;
    if(pool->used_buffers < pool->max_buffers) {
        int free;
        pool->used_buffers += 1;
        if(pool->used_buffers == pool->max_buffers)
            *last = 1;
        free = ch_msb32(pool->free_buffers);
        /* Reserve the buffer. */
        pool->free_buffers &= ~(1 << (free - 1));
        /* The msb represents the first buffer. So the value is inverted. */
        handler_buf = &pool->handlers[32 - free];
        A(handler_buf->used == 0, "Handler buffer already used.");
        handler_buf->used = 1;
        memset(&handler_buf->msg, 0, sizeof(handler_buf->msg));
        handler_buf->msg._handler = handler_buf->id;
        handler_buf->msg._flags = CH_MSG_IS_HANDLER;
        return handler_buf;
    }
    return NULL;
}

// .. c:function::
static
ch_inline
void
ch_bf_release(ch_buffer_pool_t* pool, int id)
//
//    Set given handler buffer as unused in the buffer pool structure and
//    (re-)add it to the list of free buffers.
//
//    .. todo:: Maybe use another name for this method as it does not seem to
//              return something?
//
//    :param ch_buffer_pool_t* pool: The buffer pool structure containing the
//                                   handler buffer
//    :param int id: The id of the buffer that should be marked free
//
// .. code-block:: cpp
//
{
    ch_bf_handler_t* handler_buf =  &pool->handlers[id];
    A(handler_buf->used == 1, "Double release of buffer.");
    A(pool->used_buffers > 0, "Buffer pool inconsistent.");
    A(handler_buf->id == id, "Id changed.");
    A(handler_buf->msg._handler == id, "Id changed.");
    int in_pool = pool->free_buffers & (1 << (31 - id));
    A(!in_pool, "Buffer already in pool");
    if(in_pool) {
        fprintf(
            stderr,
            "%s:%d Fatal: Double release of handler buffer. "
            "ch_buffer_pool_t:%p\n",
            __FILE__,
            __LINE__,
            (void*) pool
        );
        return;
    }
    if(!(handler_buf->msg._flags & CH_MSG_IS_HANDLER)) {
        fprintf(
            stderr,
            "%s:%d Fatal: Release of non handler message. "
            "ch_buffer_pool_t:%p\n",
            __FILE__,
            __LINE__,
            (void*) pool
        );
        return;
    }
    pool->used_buffers -= 1;
    /* Release the buffer. */
    handler_buf->used = 0;
    pool->free_buffers |= (1 << (31 - id));
}

#endif //ch_buffer_h
