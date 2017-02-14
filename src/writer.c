// ======
// Writer
// ======
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
//    Check if we have an error and cleanup
//
//    :param ch_chirp_t* chirp: Chirp instance
//    :param ch_writer_t* writer: Writer
//    :param ch_connection_t* conn: Connection
//    :param int status: Write status
//
// .. c:function::
static
ch_inline
void
_ch_wr_send(ch_connection_t* conn, ch_message_t* msg, ch_send_cb_t send_cb);
//
//    Send the message after connection has been established.
//
//    :param ch_connection_t* conn: Connection to send message
//    :param ch_message_t msg: The message to send. The memory of the message
//                             must stay valid till the callback is called.
//    :param ch_send_cb_t send_cb: Will be call
//
// .. c:function::
static
void
_ch_wr_send_actor_cb(uv_write_t* req, int status);
//
//    Called after actor is sent
//
//    :param uv_write_t* req: Write request
//    :param int status: Write status
//
// .. c:function::
static
void
_ch_wr_send_chirp_header_cb(uv_write_t* req, int status);
//
//    Called after chirp header is sent
//
//    :param uv_write_t* req: Write request
//    :param int status: Write status
//
// .. c:function::
static
void
_ch_wr_send_data_cb(uv_write_t* req, int status);
//
//    Called after data is sent
//
//    :param uv_write_t* req: Write request
//    :param int status: Write status
//
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
//    Finish up sending
//
//    :param ch_chirp_t* chirp: Chirp instance
//    :param ch_writer_t* writer: Writer
//    :param ch_connection_t* conn: Connection
//
// .. c:function::
static
void
_ch_wr_send_msg_header_cb(uv_write_t* req, int status);
//
//    Called after message header is sent
//
//    :param uv_write_t* req: Write request
//    :param int status: Write status
//
// .. c:function::
static
void
_ch_wr_send_timeout_cb(uv_timer_t* handle);
//
//    Called after send timeout. Will retry sending n-times according to
//    config.
//
//    :param uv_timer_t* handle: Timer handle to schedule callback

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
    ch_ms_message_t* net_msg = &writer->net_msg;
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
        sizeof(ch_ms_message_t),
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
