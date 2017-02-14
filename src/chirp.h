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

SGLIB_DEFINE_RBTREE_PROTOTYPES(
    ch_chirp_t,
    _left,
    _right,
    _color_field,
    SGLIB_NUMERIC_COMPARATOR
)

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
//    .. c:member:: uv_async_t close
//
//       async handler to close chirp on the main-loop
//
//    .. c:member:: int auto_start
//
//       true if we have to close the libuv loop, otherwise the loop was
//       supplied by the user
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
//    Reduce callback semaphore.
//
//    TODO params
//
// .. code-block:: cpp
//
#endif //ch_chirp_h
