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

// Declarations
// ============
//
// .. c:type:: ch_wr_flags_t
//
//    Represents writer flags. Once the writer is done and the ack is received
//    the message can be finished.
//
//    .. c:member:: CH_WR_ACK_RECEIVED
//
//       Writer has received ACK.
//
//    .. c:member:: CH_WR_WRITE_DONE
//
//       Write is done (last callback has been called).
//
//    .. c:member:: CH_WR_FAILURE
//
//       On failure we still want to finish the message, therefore failure is
//       CH_WR_ACK_RECEIVED || CH_WR_WRITE_DONE.
//
// .. code-block:: cpp
//
typedef enum {
    CH_WR_ACK_RECEIVED   = 1 << 0,
    CH_WR_WRITE_DONE     = 1 << 1,
    CH_WR_FAILURE        = CH_WR_ACK_RECEIVED | CH_WR_WRITE_DONE,
} ch_wr_flags_t;

// Direct declarations
// -------------------
//
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
//       the :c:func:`_ch_wr_write_timeout_cb` callback.
//
//    .. c:member:: ch_message_t* msg
//
//       Pointer to a message. The message is set when sending through
//       :c:func:`ch_wr_write` and being read again during the callbacks.
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
//    .. c:member:: ch_message_t* msg
//
//       Pointer to the message that should be sent after handshake.
//
//    .. c:member:: char flags
//
//       Writer flags are CH_WR_ACK_RECEIVED, CH_WR_WRITE_DONE
//
// .. code-block:: cpp
//
typedef struct ch_writer_s {
    uv_timer_t         send_timeout;
    ch_message_t*      msg;
    ch_msg_message_t   net_msg;
    ch_message_t*      send_msg;
    char               flags;
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
ch_wr_write(ch_connection_t* conn, ch_message_t* msg);
//
//    Send the message after a connection has been established.
//
//    :param ch_connection_t* conn:  Connection to send the message over.
//    :param ch_message_t msg:       The message to send. The memory of the
//                                   message must stay valid until the callback
//                                   is called.
//
//
// .. c:function::
int
ch_wr_send(ch_chirp_t* chirp, ch_message_t* msg, ch_send_cb_t send_cb);
//    Same as ch_chirp_send just for internal use.
//
//    see: :c:func:`ch_chirp_send`
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
