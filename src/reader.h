// =============
// Reader Header
// =============
//
// .. todo:: Document purpose
//
// .. code-block:: cpp
//
#ifndef ch_reader_h
#define ch_reader_h

// Project includes
// ================
//
// .. code-block:: cpp
//
#include "common.h"
#include "message.h"
#include "buffer.h"

// Declarations
// ============

// .. c:type:: ch_rd_state_t
//
//    Possible states of a reader.
//
//    .. c:member:: CH_RD_START
//
//       Initial state, chirp handshake has to be done.
//
//    .. c:member:: CH_RD_WAIT
//
//       Wait for the next message.
//
//    .. c:member:: CH_RD_HEADER
//
//       Read header.
//
//    .. c:member:: CH_RD_ACTOR
//
//       Read actor.
//
//    .. c:member:: CH_RD_DATA
//
//       Read data.
//
// .. code-block:: cpp
//
typedef enum {
    CH_RD_START     = 0,
    CH_RD_HANDSHAKE = 1,
    CH_RD_WAIT      = 2,
    CH_RD_HEADER    = 3,
    CH_RD_ACTOR     = 4,
    CH_RD_DATA      = 5,
} ch_rd_state_t;

// .. c:type:: ch_rd_handshake_t
//
//    Handshake data structure.
//
//    .. c:member:: uint16_t port
//
//       Public port which is passed to a connection on a successful handshake.
//
//    .. c:member:: uint16_t max_timeout
//
//       The maximum number of seconds to wait when connecting until a timeout
//       gets triggered. This is passed to a connection upon a successful
//       handshake. The value gets computed by the configured number of retries
//       incremented by two times the configured timeout value.
//
//    .. c:member:: uint8_t[16] identity
//
//       The identity of the remote target which is passed to a connection upon
//       a successful handshake. It is used by the connection for getting the
//       remote address.
//
// .. code-block:: cpp
//
typedef struct ch_rd_handshake_s {
    uint16_t port;
    uint16_t max_timeout;
    uint8_t  identity[CH_ID_SIZE];
} ch_rd_handshake_t;

// .. c:type:: ch_reader_t
//
//    Defines the state of a reader.
//
//    .. c:member:: ch_rd_state_t state
//
//       Current state of the reader (finite-state machine).
//
//    .. c:member:: ch_rd_handshake_t hs
//
//       Handshake data structure to send over the network, which is used as
//       data source.
//
//    .. c:member:: ch_message_t* msg
//
//       Current message
//
//    .. c:member:: ch_bf_handler_t* handler
//
//       Current handler buffer
//
//    .. c:member:: ch_message_t ack_msg
//
//       Buffer used for ack message
//
//    .. c:member:: ch_buffer_pool_t pool
//
//       Data structure containing preallocated buffers for the chirp handlers.
//
//    .. c:member:: size_t bytes_read
//
//       Counter for how many bytes were already read by the reader. This is
//       used when :c:func:`ch_rd_read` is called with a buffer of
//       :c:member:`ch_rd_read.read` bytes to read but not enough bytes are
//       being delivered over the connection :c:member:`ch_rd_read.conn`.
//
//    .. c:member:: int last
//
//       The last buffer was used (bool)
//
// .. code-block:: cpp
//
typedef struct ch_reader_s {
    ch_rd_state_t     state;
    ch_rd_handshake_t hs;
    ch_bf_handler_t*  handler;
    ch_message_t      ack_msg;
    ch_buffer_pool_t  pool;
    size_t            bytes_read;
    int               last;
} ch_reader_t;

// .. c:function::
void
ch_rd_read(ch_connection_t* conn, void* buf, size_t read);
//
//    Implements the wire protocol reader part.
//
//    :param ch_connection_t* conn: Connection the data was read from.
//    :param void* buf:             The buffer containing ``read`` bytes read.
//    :param size_t read:           The number of bytes read.

// Definitions
// ===========

// .. c:function::
static
ch_inline
void
ch_rd_free(ch_reader_t* reader)
//
//    Free the (data-) buffer pool of the given reader instance.
//
//    :param ch_reader_t* reader: The reader instance whose buffer
//                                pool shall be freed.
//
// .. code-block:: cpp
//
{
    ch_bf_free(&reader->pool);
}

// .. c:function::
static
ch_inline
ch_error_t
ch_rd_init(ch_reader_t* reader, uint8_t max_buffers)
//
//    Initialize the reader structure
//
//    :param ch_reader_t* reader: The reader instance whose buffer pool shall
//                                be initialized with ``max_buffers``.
//    :param uint8_t max_buffers: The number of buffers to allocate.
//
// .. code-block:: cpp
//
{
    reader->state = CH_RD_START;
    return ch_bf_init(&reader->pool, max_buffers);
}

#endif //ch_reader_h
