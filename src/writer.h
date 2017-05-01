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
#include "common.h"
#include "libchirp/callbacks.h"
#include "libchirp/message.h"
#include "sglib.h"

// Sglib Prototypes
// ================

// .. code-block:: cpp
//
SGLIB_DEFINE_DL_LIST_PROTOTYPES( // NOCOV
    ch_message_t,
    SGLIB_NUMERIC_COMPARATOR,
    _prev,
    _next
)

// Declarations
// ============

// Direct declarations
// -------------------
//
// .. c:type:: ch_wr_msg_cb_t
//
//    Callback called when chirp is started
//
//    .. c:member:: ch_connection_t* conn
//
//       Connection message should be sent on
//
//    .. c:member:: ch_message_t* msg
//
//       The message to send
//
// .. code-block:: cpp
//
typedef void (*ch_wr_msg_cb_t)(ch_connection_t* conn, ch_message_t* msg);

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
//    .. c:member:: ch_message_t* msg
//
//       Pointer to a message. The message is set when sending through
//       :c:func:`_ch_wr_send` and being read again during the callbacks.
//
//    .. c:member:: ch_msg_message_t net_msg
//
//       The net version of the ``msg``. The net message structure is of type
//       :c:type:`ch_msg_message_t`, which is actually
//       :c:macro:`CH_WIRE_MESSAGE`.
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
    uv_timer_t       send_timeout;
    ch_message_t*    msg;
    ch_msg_message_t net_msg;
    ch_wr_msg_cb_t   send_msg;
} ch_writer_t;

// .. c:function::
void
ch_wr_free(ch_writer_t* writer);
//
//    Free the writer data structure.
//
//    :param ch_writer_t* writer: Pointer to writer (data-) structure.
//

// .. c:function::
void
ch_wr_init(ch_writer_t* writer, ch_connection_t* conn);
//
//    Initialize the writer data structure.
//
//    Initializes the writers mutex and the libuv timer for handling timeouts
//    when sending. The connection gets set as data pointer for the sending
//    timeout.
//
//    :param ch_chirp_t* chirp:      Pointer to a chirp instance.
//    :param ch_connection_t* conn:  Pointer to a connection instance.

// .. c:function::
void
ch_wr_send(ch_connection_t* conn, ch_message_t* msg);
//
//    Send the message after a connection has been established.
//
//    :param ch_connection_t* conn:  Connection to send the message over.
//    :param ch_message_t msg:       The message to send. The memory of the
//                                   message must stay valid until the callback
//                                   is called.
//

// .. c:function::
void
_ch_wr_send_ts_cb(uv_async_t* handle);
//
//    Send all messages in the message queue.
//
//    :param uv_async_t* handle: Async handler used to trigger sending message
//                               queue.

#endif //ch_writer_h
