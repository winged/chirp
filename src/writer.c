// ======
// Writer
// ======
//
// Chirp protocol writer.
// .. todo:: Document purpose
//
// .. code-block:: cpp

// Project includes
// ================
//
// .. code-block:: cpp
//
#include "writer.h"
#include "chirp.h"
#include "protocol.h"
#include "util.h"

// Declarations
// ============

// .. c:function::
static
ch_inline
int
_ch_wr_check_send_error(
    ch_chirp_t* chirp,
    ch_writer_t* writer,
    ch_connection_t* conn,
    int status
);
//
//    Check if the given status is erroneous (that is, there was an error
//    during writing) and cleanup if necessary. Cleaning up means shutting down
//    the given connection instance, stopping the timer for the send-timeout,
//    unlocking the writer mutex and sending a protocol error
//    :c:member:`ch_error_t.CH_PROTOCOL_ERROR` callback over the connections
//    load.
//
//    :param ch_chirp_t* chirp:      Pointer to a chirp instance.
//    :param ch_writer_t* writer:    Pointer to a writer instance.
//    :param ch_connection_t* conn:  Pointer to a connection instance.
//    :param int status:             Status of the write, of type
//                                   :c:type:`ch_error_t`.
//
//    :return:                       the status, which is of type
//                                   :c:type:`ch_error_t`.
//    :rtype:                        int

// .. c:function::
static
ch_inline
void
_ch_wr_send(ch_connection_t* conn, ch_message_t* msg, ch_send_cb_t send_cb);
//
//    Send the message after a connection has been established.
//
//    :param ch_connection_t* conn:  Connection to send the message over.
//    :param ch_message_t msg:       The message to send. The memory of the
//                                   message must stay valid until the callback
//                                   is called.
//    :param ch_send_cb_t send_cb:   The callback, that will be called after
//                                   sending.

// .. c:function::
static
void
_ch_wr_send_actor_cb(uv_write_t* req, int status);
//
//    Callback which is called after an actor was written.
//
//    This callback is called, when (message-) data is being sent over a
//    connection. Upon successful sending the (message-) data, the message
//    header callback is called.
//
//    The message header callback writes the message header and calls then the
//    chirp header callback, which, in its turn, calls this callback when an
//    actor was present and written. If a message has no header, the message
//    header callback writes the actor directly --- if a such is present ---
//    and then calls this callback.
//
//    Cancels (void) if the sending was erroneous. If the sending was
//    successful and the writers message contains data, the data is being
//    written.
//
//    :param uv_write_t* req:  Write request.
//    :param int status:       Write status.

// .. c:function::
static
void
_ch_wr_send_chirp_header_cb(uv_write_t* req, int status);
//
//    Callback which is called after the messages header was written.
//
//    The successful sending of a message over a connection triggers the
//    message header callback, which, in its turn, then calls this callback ---
//    if a header is present.
//
//    Cancels (void) if the sending was erroneous. If the sending was
//    successful an actor is being written, if the writers message has an actor
//    set. Otherwise, if the writers message has data set, the data is being
//    written. If no actor and no data is set, sending is being finished.
//
//    :param uv_write_t* req:  Write request.
//    :param int status:       Write status.

// .. c:function::
static
void
_ch_wr_send_data_cb(uv_write_t* req, int status);
//
//    Callback which is called after data was written.
//
//    This callback is called after the successful write of either an actor or
//    a message header.
//
//    Cancels (void) if the writing was erroneous, finishes sending otherwise.
//
//    :param uv_write_t* req:  Write request.
//    :param int status:       Write status.

// .. c:function::
static
ch_inline
void
_ch_wr_send_finish(
    ch_chirp_t* chirp,
    ch_writer_t* writer,
    ch_connection_t* conn
);
//
//    Sends a protocol error over the connections load when acknowledging of
//    messages is disable. Does nothing if not.
//
//    If acknowledging is disabled (by configuration), the writers send-timeout
//    timer is being stopped, the writers mutex is unlocked and a protocol
//    error :c:member:`ch_error_t.CH_PROTOCOL_ERROR` callback over the
//    connections load is being sent.
//
//    :param ch_chirp_t* chirp:      Pointer to a chirp instance.
//    :param ch_writer_t* writer:    Pointer to a writer instance.
//    :param ch_connection_t* conn:  Pointer to a connection instance.

// .. c:function::
static
void
_ch_wr_send_msg_header_cb(uv_write_t* req, int status);
//
//    Callback which is called after the messages was successfully written.
//
//    Cancels (void) if the writing was erroneous, finishes sending otherwise.
//    If the message (coming from the writer) has a header set, the header is
//    written. Otherwise, if the message has an actor set, the actor is being
//    written. Else, if the message has data attached, that data is being
//    written. In all other cases the sending is being finished using
//    :c:func:`_ch_wr_send_finish`.
//
//    :param uv_write_t* req:  Write request.
//    :param int status:       Write status.

// .. c:function::
static
void
_ch_wr_send_timeout_cb(uv_timer_t* handle);
//
//    Callback which is called after the writer reaches its timeout for
//    sending. The timeout is set by the chirp configuration and is 5 seconds
//    by default. When this callback is called, the connection is being shut
//    down, the writers mutex is being unlocked and a timeout error
//    :c:member:`ch_error_t.CH_TIMEOUT` is being sent over the connections
//    load.
//
//    :param uv_timer_t* handle: Pointer to a timer handle to schedule
//                               callback.

// Definitions
// ===========

// .. c:function::
static
ch_inline
int
_ch_wr_check_send_error(
    ch_chirp_t* chirp,
    ch_writer_t* writer,
    ch_connection_t* conn,
    int status
)
//    :noindex:
//
//    see: :c:func:`_ch_wr_check_send_error`
//
// .. code-block:: cpp
//
{
    if(status != CH_SUCCESS) {
        L(
            chirp,
            "Write failed with uv status: %d. ch_chirp_t:%p, "
            "ch_connection_t:%p",
            status,
            (void*) chirp,
            (void*) conn
        );
        ch_cn_shutdown(conn);
        uv_timer_stop(&writer->send_timeout);
        uv_mutex_unlock(&writer->lock);
        writer->send_cb(CH_PROTOCOL_ERROR, conn->load);
        return CH_PROTOCOL_ERROR;
    }
    return CH_SUCCESS;
}

// .. c:function::
static
ch_inline
void
_ch_wr_send(ch_connection_t* conn, ch_message_t* msg, ch_send_cb_t send_cb)
//    :noindex:
//
//    see: :c:func:`_ch_wr_send`
//
// .. code-block:: cpp
//
{
    int tmp_err;

    ch_chirp_t* chirp = conn->chirp;
    A(chirp->_init == CH_CHIRP_MAGIC, "Not a ch_chirp_t*");
    ch_chirp_int_t* ichirp = chirp->_;
    ch_writer_t* writer = &conn->writer;
    // We have to lock before starting the timeout. Otherwise the
    // lock-wait-time would count towards the timeout.
    uv_mutex_lock(&writer->lock);
    // Use the writers net message structure to write the actual message over
    // the connection. The net message structure is of type
    // :c:type:`ch_msg_message_t`, which is actually :c:macro:`CH_WIRE_MESSAGE`.
    // The difference between ``msg`` and ``net_msg`` is, that ``msg`` is of
    // type :c:type:`ch_message_t` and ``net_msg`` of type
    // :c:macro:`CH_WIRE_MESSAGE`. That means ``net_msg`` is stripped down to
    // essentially only the identity, the serial number, the message type and
    // the lengths of the header, the actor and the data.
    ch_msg_message_t* net_msg = &writer->net_msg;
    writer->msg = msg;
    writer->send_cb = send_cb;
    tmp_err = uv_timer_start(
        &writer->send_timeout,
        _ch_wr_send_timeout_cb,
        ichirp->config.TIMEOUT * 1000,
        0
    );
    if(tmp_err != CH_SUCCESS) {
        E(
            chirp,
            "Starting send timeout failed: %d. ch_connection_t:%p,"
            " ch_chirp_t:%p",
            tmp_err,
            (void*) conn,
            (void*) chirp
        );
    }
    memcpy(
        net_msg->serial,
        msg->serial,
        sizeof(net_msg->serial)
    );
    memcpy(
        net_msg->identity,
        msg->identity,
        sizeof(net_msg->identity)
    );
    net_msg->header_len = htons(msg->header_len);
    net_msg->actor_len  = htons(msg->actor_len);
    net_msg->data_len   = htonl(msg->data_len);
    ch_cn_write(
        conn,
        net_msg,
        sizeof(ch_msg_message_t),
        _ch_wr_send_msg_header_cb
    );
}

// .. c:function::
static
void
_ch_wr_send_actor_cb(uv_write_t* req, int status)
//    :noindex:
//
//    see: :c:func:`_ch_wr_send_actor_cb`
//
// .. code-block:: cpp
//
{
    ch_connection_t* conn = req->data;
    ch_chirp_t* chirp = conn->chirp;
    A(chirp->_init == CH_CHIRP_MAGIC, "Not a ch_chirp_t*");
    ch_writer_t* writer = &conn->writer;
    ch_message_t* msg = writer->msg;
    if(_ch_wr_check_send_error(chirp, writer, conn, status)) return;
    if(msg->data_len > 0)
        ch_cn_write(
            conn,
            msg->data,
            msg->data_len,
            _ch_wr_send_data_cb
        );
    else
        _ch_wr_send_finish(chirp, writer, conn);
}

// .. c:function::
static
void
_ch_wr_send_data_cb(uv_write_t* req, int status)
//    :noindex:
//
//    see: :c:func:`_ch_wr_send_data_cb`
//
// .. code-block:: cpp
//
{
    ch_connection_t* conn = req->data;
    ch_chirp_t* chirp = conn->chirp;
    A(chirp->_init == CH_CHIRP_MAGIC, "Not a ch_chirp_t*");
    ch_writer_t* writer = &conn->writer;
    if(_ch_wr_check_send_error(chirp, writer, conn, status)) return;
    _ch_wr_send_finish(chirp, writer, conn);
}

// .. c:function::
static
ch_inline
void
_ch_wr_send_finish(
    ch_chirp_t* chirp,
    ch_writer_t* writer,
    ch_connection_t* conn
)
//    :noindex:
//
//    see: :c:func:`_ch_wr_send_finish`
//
// .. code-block:: cpp
//
{
    ch_chirp_int_t* ichirp  = chirp->_;
    if(ichirp->config.ACKNOWLEDGE == 0) {
        uv_timer_stop(&writer->send_timeout);
        uv_mutex_unlock(&writer->lock);
        writer->send_cb(CH_PROTOCOL_ERROR, conn->load);
    }
}

// .. c:function::
static
void
_ch_wr_send_chirp_header_cb(uv_write_t* req, int status)
//    :noindex:
//
//    see: :c:func:`_ch_wr_send_chirp_header_cb`
//
// .. code-block:: cpp
//
{
    ch_connection_t* conn = req->data;
    ch_chirp_t* chirp = conn->chirp;
    A(chirp->_init == CH_CHIRP_MAGIC, "Not a ch_chirp_t*");
    ch_writer_t* writer = &conn->writer;
    ch_message_t* msg = writer->msg;
    if(_ch_wr_check_send_error(chirp, writer, conn, status)) return;
    if(msg->actor_len > 0)
        ch_cn_write(
            conn,
            msg->actor,
            msg->actor_len,
            _ch_wr_send_actor_cb
        );
    else if(msg->data_len > 0)
        ch_cn_write(
            conn,
            msg->data,
            msg->data_len,
            _ch_wr_send_data_cb
        );
    else
        _ch_wr_send_finish(chirp, writer, conn);
}

// .. c:function::
static
void
_ch_wr_send_msg_header_cb(uv_write_t* req, int status)
//    :noindex:
//
//    see: :c:func:`_ch_wr_send_msg_header_cb`
//
// .. code-block:: cpp
//
{
    ch_connection_t* conn = req->data;
    ch_chirp_t* chirp = conn->chirp;
    A(chirp->_init == CH_CHIRP_MAGIC, "Not a ch_chirp_t*");
    ch_writer_t* writer = &conn->writer;
    ch_message_t* msg = writer->msg;
    if(_ch_wr_check_send_error(chirp, writer, conn, status)) return;
    if(msg->header_len > 0)
        ch_cn_write(
            conn,
            msg->header,
            msg->header_len,
            _ch_wr_send_chirp_header_cb
        );
    else if(msg->actor_len > 0)
        ch_cn_write(
            conn,
            msg->actor,
            msg->actor_len,
            _ch_wr_send_actor_cb
        );
    else if(msg->data_len > 0)
        ch_cn_write(
            conn,
            msg->data,
            msg->data_len,
            _ch_wr_send_data_cb
        );
    else
        _ch_wr_send_finish(chirp, writer, conn);
}

// .. c:function::
static
void
_ch_wr_send_timeout_cb(uv_timer_t* handle)
//    :noindex:
//
//    see: :c:func:`_ch_wr_send_timeout_cb`
//
// .. code-block:: cpp
//
{
    ch_connection_t* conn = handle->data;
    ch_writer_t* writer = &conn->writer;
    ch_chirp_t* chirp = conn->chirp;
    A(chirp->_init == CH_CHIRP_MAGIC, "Not a ch_chirp_t*");
    L(
        chirp,
        "Write timed out. ch_chirp_t:%p, ch_connection_t:%p",
        (void*) chirp,
        (void*) conn
    );
    ch_cn_shutdown(conn);
    uv_timer_stop(&writer->send_timeout);
    uv_mutex_unlock(&writer->lock);
    writer->send_cb(CH_TIMEOUT, conn->load);
}

//
// .. c:function::
void
ch_chirp_send(ch_chirp_t* chirp, ch_message_t* msg, ch_send_cb_t send_cb)
//    :noindex:
//
//    see: :c:func:`ch_chirp_send`
//
// .. code-block:: cpp
//
{
    ch_connection_t search_conn;
    ch_connection_t* conn;

    A(chirp->_init == CH_CHIRP_MAGIC, "Not a ch_chirp_t*");
    ch_chirp_int_t* ichirp  = chirp->_;
    ch_protocol_t* protocol = &ichirp->protocol;
    search_conn.ip_protocol = msg->ip_protocol;
    search_conn.port        = msg->port;
    memcpy(
        search_conn.address,
        msg->address,
        msg->ip_protocol == CH_IPV6 ? 16 : 4
    );
    conn = sglib_ch_connection_t_find_member(
        protocol->connections,
        &search_conn
    );
    ch_random_ints_as_bytes(msg->serial, sizeof(msg->serial));
    if(conn == NULL) {
        // TODO connect
    } else
        _ch_wr_send(conn, msg, send_cb);
}

// .. c:function::
void
ch_wr_init(ch_writer_t* writer, ch_connection_t* conn)
//    :noindex:
//
//    see: :c:func:`ch_wr_init`
//
// .. code-block:: cpp
//
{
    int tmp_err;

    ch_chirp_t* chirp = conn->chirp;
    A(chirp->_init == CH_CHIRP_MAGIC, "Not a ch_chirp_t*");
    ch_chirp_int_t* ichirp = chirp->_;
    uv_mutex_init(&writer->lock);
    tmp_err = uv_timer_init(ichirp->loop, &writer->send_timeout);
    if(tmp_err != CH_SUCCESS) {
        E(
            chirp,
            "Initializing send timeout failed: %d. ch_chirp_t:%p",
            tmp_err,
            (void*) chirp
        );
    }
    writer->send_timeout.data = conn;
}
