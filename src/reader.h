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
#include "serializer.h"

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
    CH_RD_DATA      = 4,
} ch_rd_state_t;

// .. c:type:: ch_reader_t
//
//    Defines the state of a reader.
//
//    .. c:member:: ch_rd_state_t state
//
//       Current state of the reader (finite-state machine).
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
//    .. c:member:: size_t bytes_read
//
//       Counter for how many bytes were already read by the reader. This is
//       used when :c:func:`ch_rd_read` is called with a buffer of
//       :c:member:`ch_rd_read.read` bytes to read but not enough bytes are
//       being delivered over the connection :c:member:`ch_rd_read.conn`.
//
//    .. c:member:: int last_handler
//
//       The last handler (buffer) was used (bool)
//
// .. code-block:: cpp
//
typedef struct ch_reader_s {
    ch_rd_state_t    state;
    ch_bf_handler_t* handler;
    ch_message_t     ack_msg;
    size_t           bytes_read;
    int              last_handler;
    ch_buf           net_msg[CH_SR_WIRE_MESSAGE_SIZE];
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
inline
void
ch_rd_init(ch_reader_t* reader)
//
//    Initialize the reader structure
//
//    :param ch_reader_t* reader: The reader instance whose buffer pool shall
//                                be initialized with ``max_buffers``.
// .. code-block:: cpp
//
{
    reader->state = CH_RD_START;
}

#endif //ch_reader_h
