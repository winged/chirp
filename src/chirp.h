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
//   * Up to 50'000 msg/s on a single connection
//   * Up to TODO msg/s in star topology
//   * Using multiple channels multiplies throughput until another bottle-neck
//     kicks in
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
// .. code-block:: cpp
//
struct ch_chirp_int_s {
    ch_config_t     config;
    int             closing_tasks;
    uint8_t         flags;
    uv_async_t      close;
    uv_async_t      start;
    ch_start_cb_t   start_cb;
    uv_signal_t     signals[2];
    uv_prepare_t    close_check;
    ch_protocol_t   protocol;
    ch_encryption_t encryption;
    uv_loop_t*      loop;
    uint8_t         identity[CH_ID_SIZE];
    uint16_t        public_port;
};

// .. c:function::
void
ch_chirp_close_cb(uv_handle_t* handle);
//
//    Reduce closing callback semaphore.
//
//    :param uv_handle_t* handle: A libuv handle containing the chirp object
//
// .. code-block:: cpp
//
#endif //ch_chirp_h
