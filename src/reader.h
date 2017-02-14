// =============
// Reader Header
// =============
//
// .. code-block:: cpp

#ifndef ch_reader_h
#define ch_reader_h

#include "chirp/common.h"
#include "message.h"
#include "buffer.h"

struct ch_connection_s;

// .. c:type:: ch_rd_state_t
//
//    Represents connection flags.
//
//    .. c:member:: CH_RD_START
//
//       Initial state, chirp handshake has to be done
//
//    .. c:member:: CH_RD_WAIT
//
//       Wait for the next message
//
//    .. c:member:: CH_RD_HEADER
//
//       Read header
//
//    .. c:member:: CH_RD_ACTOR
//
//       Read actor
//
//    .. c:member:: CH_RD_DATA
//
//       Read data
//
// .. code-block:: cpp
//
typedef enum {
    CH_RD_START     = 0,
    CH_RD_HANDSHAKE = 1,
    CH_RD_WAIT      = 2,
    CH_RD_HEADER    = 3,
    CH_RD_ACTOR     = 4,
    CH_RD_DATA      = 5
} ch_rd_state_t;

// .. c:type:: ch_rd_handshake_t
//
//    Connection handshake data
//
//    .. c:member:: port
//
//    Public port
//
//    .. c:member:: port
//
//    Maximum timeout
//
//    .. c:member:: identity
//
//    identity
//
// .. code-block:: cpp

typedef struct ch_rd_handshake_s {
    uint16_t      port;
    uint16_t      max_timeout;
    unsigned char identity[16];
} ch_rd_handshake_t;

// .. c:type:: ch_reader_t
//
//    Contains the state of the reader
//
//    .. c:member:: unsinged char state
//
//    Statemachine-state of the reader
//
//    .. c:member:: ch_rd_handshake_t hs
//
//    Handshake structure to send over the network
//
//    .. c:member:: ch_rd_message_t msg
//
//    Wire protocol message in network order
//
// .. code-block:: cpp

typedef struct ch_reader_s {
    ch_rd_state_t     state;
    ch_rd_handshake_t hs;
    ch_ms_message_t   msg;
    ch_buffer_pool_t  pool;
    size_t            bytes_read;
} ch_reader_t;

// .. c:function::
static
ch_inline
void
ch_rd_free(ch_reader_t* reader)
//
//    Initialize the reader structure
//
//    :param ch_reader_t* reader: The reader
//
// .. code-block:: cpp
//
{
    ch_bf_free(&reader->pool);
}

// .. c:function::
static
ch_inline
void
ch_rd_init(ch_reader_t* reader, uint8_t max_buffers)
//
//    Initialize the reader structure
//
//    :param ch_reader_t* reader: The reader
//    :param max_buffers: Buffers to allocate
//
// .. code-block:: cpp
//
{
    reader->state = CH_RD_START;
    ch_bf_init(&reader->pool, max_buffers);
}

// .. c:function::
void
ch_rd_read(struct ch_connection_s* conn, void* buf, size_t read);
//
//    Implements the wire protocol reader part.
//
//    :param ch_connection_t* conn: Connection the data was read from
//    :param void* buf: Buffer containing bytes read
//    :param size_t read: Count of bytes read
//
// .. code-block:: cpp

#endif //ch_reader_h
