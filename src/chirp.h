// ============
// Chirp header
// ============
//
// .. code-block:: cpp

#ifndef ch_chirp_h
#define ch_chirp_h

#include "libchirp.h"
#include "protocol.h"
#include "encryption.h"

#include "sglib.h"

// Sglib Prototypes
// ================

// .. code-block:: cpp
//
SGLIB_DEFINE_RBTREE_PROTOTYPES( // NOCOV
    ch_chirp_t,
    _left,
    _right,
    _color_field,
    SGLIB_NUMERIC_COMPARATOR
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
//    .. c:member:: uv_async_t close
//
//       Asynchronous handler to close chirp on the main-loop.
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
    uv_prepare_t    close_check;
    ch_protocol_t   protocol;
    ch_encryption_t encryption;
    uv_loop_t*      loop;
    uint8_t         identity[16];
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
