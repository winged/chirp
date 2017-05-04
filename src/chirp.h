// ============
// Chirp header
// ============
//
// Features
// ========
//
// * Fully automatic connection setup
//
// * TLS support
//
//   * Connections to 127.0.0.1 and ::1 aren't encrypted
//
// * Flow control
//
//   * Chirp won't overload peers out-of-the box, if you work with long
//     requests >2.5s adjust the timeout
//   * Peer-load is reported so you can implement load-balancing easily
//
// * Easy message routing
//
// * Robust
//
//   * No message can be lost without an error (or it is a bug)
//   * Due to retry it takes a very bad network for messages to be lost
//
// * Very thin API
//
// * Minimal code-base, all additional features will be implemented as modules
//   in an upper layer
//
// * Fast
//
//   * Up to 50'000 msg/s on a single-connection (encrypted 35'000 msg/s)
//   * Up to 100'000 msg/s in star-topology (encrypted same)
//
//     * Which shows that chirp is highly optimized, but still if the network
//       delay is bigger star-topology is the way to go.
//
// .. code-block:: cpp
//
#ifndef ch_chirp_h
#define ch_chirp_h

// Project includes
// ================
//
// .. code-block:: cpp
//
#include "libchirp.h"
#include "protocol.h"
#include "encryption.h"

// System includes
// ===============
//
// .. code-block:: cpp
//
#include "sglib.h"

typedef ch_message_t ch_message_dest_t;
typedef ch_message_t ch_message_send_t;
typedef ch_message_t ch_message_queue_t;

// Sglib Prototypes
// ================

// .. code-block:: cpp
//
#define CH_MESSAGE_DEST_CMP(x,y) ch_message_dest_cmp(x, y)

// .. code-block:: cpp
//
SGLIB_DEFINE_RBTREE_PROTOTYPES( // NOCOV
    ch_message_dest_t,
    _left,
    _right,
    _color_field,
    CH_MESSAGE_DEST_CMP
)

// Declarations
// ============

// .. c:type:: ch_chirp_flags_t
//
//    Represents chirps flags.
//
//    .. c:member:: CH_CHIRP_AUTO_STOP
//
//       Stop the loop on closing. This is useful if the loop is only used by
//       chirp.
//
//    .. c:member:: CH_CHIRP_CLOSED
//
//       Chirp is closed.
//
//    .. c:member:: CH_CHIRP_CLOSING
//
//       Chirp is being closed.
//
// .. code-block:: cpp
//
typedef enum {
    CH_CHIRP_AUTO_STOP   = 1 << 0,
    CH_CHIRP_CLOSED      = 1 << 1,
    CH_CHIRP_CLOSING     = 1 << 2,
} ch_chirp_flags_t;

// .. c:type:: ch_chirp_int_t
//
//    Chirp object.
//
//    .. c:member:: ch_config_t config
//
//       The current chirp configuration.
//
//    .. c:member:: int closing_tasks
//
//       Counter for the number of tasks when closing a connection (e.g.
//       shutdown). This acts as semaphore.
//
//    .. c:member:: uint8_t flags
//
//       Holds the flags from :c:type:`ch_chirp_flags_t`, hence indicates
//       whether chirp is closing, already closed and if the loop shall be
//       stopped when closing.
//
//    .. c:member:: uv_async_t start
//
//       Asynchronous handler called when chirp is ready.
//
//    .. c:member:: ch_start_cb_t start_cb
//
//       User supplied callback called when chirp is ready.
//
//    .. c:member:: uv_async_t close
//
//       Asynchronous handler to close chirp on the main-loop.
//
//    .. c:member:: uv_signal_t signals[2]
//
//       Libuv handles for managing unix signals. We currently register two
//       handlers, one for SIGINT, one for SIGTERM.
//
//    .. c:member:: uv_prepare_t close_check
//
//       Handle which will run the given callback (close callback, closes chirp
//       when the closing semaphore reaches zero) once per loop iteration.
//
//    .. c:member:: ch_protocol_t protocol
//
//       Reference to protocol object. Provides access to connection and data
//       specific functions.
//
//    .. c:member:: ch_encryption_t encryption
//
//       Reference to encryption object. Is used when a encrypted connection is
//       used.
//
//    .. c:member:: uv_loop_t* loop
//
//       Pointer to the libuv (main) event loop. The event loop is the central
//       part of libuv’s functionality. It takes care of polling for i/o and
//       scheduling callbacks to be run based on different sources of events.
//
//    .. c:member:: uint8_t identity[16]
//
//       Array holding the identity of this chirp configuration.
//
//    .. c:member:: uint16_t public_port
//
//       The public port which this chirp configuration uses for connections.
//
//    .. c:member:: ch_message_t* send_ts_queue
//
//       The send queue for ch_chirp_send_ts
//
//    .. c:member:: ch_message_t* send_ts_queue_end
//
//       The end of send queue for ch_chirp_send_ts
//
//    .. c:member:: uv_async_t send_ts
//
//       Async event for waking up _ch_wr_send_ts_cb
//
//    .. c:member:: uv_mutex_t send_ts_queue_lock
//
//       Mutex to lock the send_ts_queue
//
//    .. c:member:: ch_message_dest_t* message_queue;
//
//       Contains queued messages by destination
//
// .. code-block:: cpp
//
struct ch_chirp_int_s {
    ch_config_t         config;
    int                 closing_tasks;
    uint8_t             flags;
    uv_async_t          close;
    uv_async_t          start;
    ch_start_cb_t       start_cb;
    uv_signal_t         signals[2];
    uv_prepare_t        close_check;
    ch_protocol_t       protocol;
    ch_encryption_t     encryption;
    uv_loop_t*          loop;
    uint8_t             identity[CH_ID_SIZE];
    uint16_t            public_port;
    ch_message_t*       send_ts_queue;
    ch_message_t*       send_ts_queue_end;
    uv_async_t          send_ts;
    uv_mutex_t          send_ts_queue_lock;
    ch_message_dest_t*  message_queue;
};

// .. c:function::
void
ch_chirp_close_cb(uv_handle_t* handle);
//
//    Reduce closing callback semaphore.
//
//    :param uv_handle_t* handle: A libuv handle containing the chirp object
//

// .. c:function::
void
ch_chirp_message_finish(
        ch_chirp_t* chirp,
        ch_message_t* msg,
        int status,
        float load
);
//
//    Call the user callback and then check the message queue for further
//    messages and send them.
//
//    :param ch_chirp_t* chirp: Chirp instance
//    :param ch_message_t* msg: Pointer to the message
//    :param int status: Error code
//    :param float load: The load of the remote peer
//

// Definitions
// ===========

// .. c:function::
static
ch_inline
int
ch_message_dest_cmp(ch_message_dest_t* x, ch_message_dest_t* y)
//
//    Compare operator for messages.
//
//    :param ch_message_dest_t* x: First connection instance to compare
//    :param ch_message_dest_t* y: Second connection instance to compare
//
//    :return: the comparision between
//                 - the IP protocols, if they are not the same, or
//                 - the addresses, if they are not the same, or
//                 - the ports
//    :rtype: int
//
// .. code-block:: cpp
//
{
    if(x->ip_protocol != y->ip_protocol) {
        return x->ip_protocol - y->ip_protocol;
    } else {
        int tmp_cmp = memcmp(
            x->address,
            y->address,
            x->ip_protocol == CH_IPV6 ? CH_IP_ADDR_SIZE : CH_IP4_ADDR_SIZE
        );
        if(tmp_cmp != 0) {
            return tmp_cmp;
        } else {
            return x->port - y->port;
        }
    }
}

#endif //ch_chirp_h
