// =============
// Writer Header
// =============
//
// Chirp protocol writer
// .. todo:: Document purpose
//
// .. code-block:: cpp
//
#ifndef ch_writer_h
#define ch_writer_h

// Project includes
// ================
//
// .. code-block:: cpp
//
#include "libchirp/common.h"
#include "libchirp/callbacks.h"
#include "libchirp/message.h"

// Declarations
// ============

// Forward declarations
// --------------------

struct ch_connection_s;

// Direct declarations
// -------------------

// .. c:type:: ch_writer_t
//
//    The chirp protocol writer data strcture.
//
//    .. c:member:: ch_send_cb_t send_cb
//
//       Callback that will be called upon successful sending of a message.
//
//    .. c:member:: uv_timer_t send_timeout
//
//       Libuv timer handle for setting a timeout when trying to send a
//       message. At the end of the defined timeout time, the timer triggers
//       the :c:func:`_ch_wr_send_timeout_cb` callback.
//
//    .. c:member:: uv_mutex_t lock
//
//       Mutex acting as lock. The mutex is locked when a writer is initialized
//       or when a writer is trying to send a message. It is unlocked when
//       sending is finished, when an error during sending happened or when
//       sending runs into a timeout.
//
//    .. c:member:: ch_message_t* msg
//
//       Pointer to a message. The message is set when sending through
//       :c:func:`_ch_wr_send` and being read again during the callbacks.
//
//    .. c:member:: ch_ms_message_t net_msg
//
//       The net version of the ``msg``. The net message structure is of type
//       :c:type:`ch_ms_message_t`, which is actually :c:macro:`CH_WIRE_MESSAGE`.
//
//       The difference between ``msg`` and ``net_msg`` is, that ``msg`` is of
//       type :c:type:`ch_message_t` and ``net_msg`` of type
//       :c:macro:`CH_WIRE_MESSAGE`. That means ``net_msg`` is stripped down to
//       essentially only the identity, the serial number, the message type and
//       the lengths of the header, the actor and the data.
//
// .. code-block:: cpp
//
typedef struct ch_writer_s {
    ch_send_cb_t    send_cb;
    uv_timer_t      send_timeout;
    uv_mutex_t      lock;
    ch_message_t*   msg;
    ch_ms_message_t net_msg;
} ch_writer_t;

// .. c:function::
static
ch_inline
void
ch_wr_free(ch_writer_t* writer)
//
//    Free the writer data structure. This means essentially destroying the
//    muteck on the writers lock attribute :c:member:`ch_writer_t.lock`.
//
//    :param ch_writer_t* writer: Pointer to writer (data-) structure.
//
// .. code-block:: cpp
//
{
    uv_mutex_destroy(&writer->lock);
}

// .. c:function::
void
ch_wr_init(ch_writer_t* writer, struct ch_connection_s* conn);
//
//    Initialize the writer data structure.
//
//    Initializes the writers mutex and the libuv timer for handling timeouts
//    when sending. The connection gets set as data pointer for the sending
//    timeout.
//
//    :param ch_chirp_t* chirp:      Pointer to a chirp instance.
//    :param ch_connection_t* conn:  Pointer to a connection instance.

#endif //ch_writer_h
