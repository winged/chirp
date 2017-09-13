// ======
// Writer
// ======
//

// Project includes
// ================
//
// .. code-block:: cpp
//
#include "writer.h"
#include "chirp.h"
#include "protocol.h"
#include "util.h"
#include "remote.h"

// Declarations
// ============

// .. c:function::
static
ch_inline
int
_ch_wr_check_write_error(
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
//

// .. c:function::
static
void
_ch_wr_close_failed_conn_cb(uv_handle_t* handle);
//
//    Called by libuv when failed connection is close.
//
//    :param uv_handle_t* handle: The libuv handle holding the
//                                connection


// .. c:function::
static
void
_ch_wr_connect_cb(uv_connect_t* req, int status);
//
//    Called by libuv after trying to connect. Contains the connection status.
//
//    :param uv_connect_t* req: Connect request, containing the connection.
//    :param int status:        Status of the connection.
//

// .. c:function::
static
void
_ch_wr_write_chirp_header_cb(uv_write_t* req, int status);
//
//    Callback which is called after the messages header was written.
//
//    The successful sending of a message over a connection triggers the
//    message header callback, which, in its turn, then calls this callback ---
//    if a header is present.
//
//    Cancels (void) if the sending was erroneous. Next data will be written if
//    the message has data.
//
//    :param uv_write_t* req:  Write request.
//    :param int status:       Write status.

// .. c:function::
static
void
_ch_wr_write_data_cb(uv_write_t* req, int status);
//
//    Callback which is called after data was written.
//
//    Cancels (void) if the writing was erroneous, finishes sending otherwise.
//
//    :param uv_write_t* req:  Write request.
//    :param int status:       Write status.

// .. c:function::
static
ch_inline
void
_ch_wr_write_finish(
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
_ch_wr_write_msg_header_cb(uv_write_t* req, int status);
//
//    Callback which is called after the messages was successfully written.
//
//    Cancels (void) if the writing was erroneous, finishes sending otherwise.
//    If the message (coming from the writer) has a header set, the header is
//    written. Otherwise, if the message has data attached, that data is being
//    written. In all other cases the sending is being finished using
//    :c:func:`_ch_wr_write_finish`.
//
//    :param uv_write_t* req:  Write request.
//    :param int status:       Write status.

// .. c:function::
static
void
_ch_wr_write_timeout_cb(uv_timer_t* handle);
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
_ch_wr_check_write_error(
        ch_chirp_t* chirp,
        ch_writer_t* writer,
        ch_connection_t* conn,
        int status
)
//    :noindex:
//
//    see: :c:func:`_ch_wr_check_write_error`
//
// .. code-block:: cpp
//
{
    A(writer->msg != NULL, "ⱳriter->msg should be set on callback")
    if(status != CH_SUCCESS) {
        LC(
            chirp,
            "Write failed with uv status: %d. ", "ch_connection_t:%p",
            status,
            (void*) conn
        );
        ch_cn_shutdown(conn, CH_PROTOCOL_ERROR);
        uv_timer_stop(&writer->send_timeout);
        return CH_PROTOCOL_ERROR;
    }
    return CH_SUCCESS;
}

// .. c:function::
static
void _ch_wr_close_failed_conn_cb(uv_handle_t* handle)
//    :noindex:
//
//    see: :c:func:`_ch_wr_close_failed_conn_cb`
//
// .. code-block:: cpp
//
{
    ch_message_t* msg = handle->data;
    ch_chirp_t* chirp = msg->chirp;
    A(chirp->_init == CH_CHIRP_MAGIC, "Not a ch_chirp_t*");
    ch_connection_t* conn = msg->_conn;
    ch_free(conn);
    msg->_flags &= ~CH_MSG_USED;
    if(msg->_send_cb != NULL) {
        /* The user may free the message in the cb */
        ch_send_cb_t cb = msg->_send_cb;
        msg->_send_cb = NULL;
        cb(msg, CH_CANNOT_CONNECT, -1);
    }
}

// .. c:function::
void
_ch_wr_connect_cb(uv_connect_t* req, int status)
//    :noindex:
//
//    see: :c:func:`_ch_wr_connect_cb`
//
// .. code-block:: cpp
//
{
    ch_text_address_t taddr;
    ch_message_t* msg = req->data;
    ch_chirp_t* chirp = msg->chirp;
    A(chirp->_init == CH_CHIRP_MAGIC, "Not a ch_chirp_t*");
    ch_connection_t* conn = msg->_conn;
    ch_msg_get_address(msg, &taddr);
    if(status == CH_SUCCESS) {
        LC(
            chirp,
            "Connected to remote %s:%d. ", "ch_connection_t:%p",
            taddr.data,
            msg->port,
            (void*) conn
        );
        /* Here we join the code called on accept. */
        ch_pr_conn_start(chirp, conn, &conn->client, 0);
    } else {
        EC(
            chirp,
            "Connection to remote failed %s:%d (%d). ",
            "ch_connection_t:%p",
            taddr.data,
            msg->port,
            status,
            (void*) conn
        );
        conn->client.data = msg;
        uv_close((uv_handle_t*) &conn->client, _ch_wr_close_failed_conn_cb);
    }
}

// .. c:function::
void
_ch_wr_send_ts_cb(uv_async_t* handle)
//    :noindex:
//
//    see: :c:func:`_ch_wr_send_ts_cb`
//
// .. code-block:: cpp
//
{
    ch_chirp_t* chirp = handle->data;
    A(chirp->_init == CH_CHIRP_MAGIC, "Not a ch_chirp_t*");
    ch_chirp_int_t* ichirp = chirp->_;
    uv_mutex_lock(&ichirp->send_ts_queue_lock);

    ch_message_t* cur;
    ch_msg_dequeue(&ichirp->send_ts_queue, &cur);
    while(cur != NULL) {
        ch_chirp_send(chirp, cur, cur->_send_cb);
        ch_msg_dequeue(&ichirp->send_ts_queue, &cur);
    }
    uv_mutex_unlock(&ichirp->send_ts_queue_lock);
}

// .. c:function::
static
void
_ch_wr_write_chirp_header_cb(uv_write_t* req, int status)
//    :noindex:
//
//    see: :c:func:`_ch_wr_write_chirp_header_cb`
//
// .. code-block:: cpp
//
{
    ch_connection_t* conn = req->data;
    ch_chirp_t* chirp = conn->chirp;
    A(chirp->_init == CH_CHIRP_MAGIC, "Not a ch_chirp_t*");
    ch_writer_t* writer = &conn->writer;
    ch_message_t* msg = writer->msg;
    if(_ch_wr_check_write_error(chirp, writer, conn, status)) return;
    if(msg->data_len > 0)
        ch_cn_write(
            conn,
            msg->data,
            msg->data_len,
            _ch_wr_write_data_cb
        );
    else
        _ch_wr_write_finish(chirp, writer, conn);
}

// .. c:function::
static
void
_ch_wr_write_data_cb(uv_write_t* req, int status)
//    :noindex:
//
//    see: :c:func:`_ch_wr_write_data_cb`
//
// .. code-block:: cpp
//
{
    ch_connection_t* conn = req->data;
    ch_chirp_t* chirp = conn->chirp;
    A(chirp->_init == CH_CHIRP_MAGIC, "Not a ch_chirp_t*");
    ch_writer_t* writer = &conn->writer;
    if(_ch_wr_check_write_error(chirp, writer, conn, status)) return;
    _ch_wr_write_finish(chirp, writer, conn);
}

// .. c:function::
static
ch_inline
void
_ch_wr_write_finish(
        ch_chirp_t* chirp,
        ch_writer_t* writer,
        ch_connection_t* conn
)
//    :noindex:
//
//    see: :c:func:`_ch_wr_write_finish`
//
// .. code-block:: cpp
//
{
    ch_chirp_int_t* ichirp  = chirp->_;
    ch_message_t* msg = writer->msg;
    A(msg != NULL, "Writer has no message");
    if((
            ichirp->config.ACKNOWLEDGE == 0 ||
            !(msg->type & CH_MSG_REQ_ACK)
    )) {
        uv_timer_stop(&writer->send_timeout);
        msg->_flags |= CH_MSG_ACK_RECEIVED; /* Emulate ACK */
    }
    msg->_flags |= CH_MSG_WRITE_DONE;
    writer->msg = NULL;
    ch_chirp_try_message_finish(
        chirp,
        conn,
        msg,
        CH_SUCCESS,
        conn->load
    );
}

// .. c:function::
static
void
_ch_wr_write_msg_header_cb(uv_write_t* req, int status)
//    :noindex:
//
//    see: :c:func:`_ch_wr_write_msg_header_cb`
//
// .. code-block:: cpp
//
{
    ch_connection_t* conn = req->data;
    ch_chirp_t* chirp = conn->chirp;
    A(chirp->_init == CH_CHIRP_MAGIC, "Not a ch_chirp_t*");
    ch_writer_t* writer = &conn->writer;
    ch_message_t* msg = writer->msg;
    if(_ch_wr_check_write_error(chirp, writer, conn, status)) return;
    if(msg->header_len > 0) // TODO: && !(msg->_flags & CH_MSG_SENT_HEADER))
        ch_cn_write(
            conn,
            msg->header,
            msg->header_len,
            _ch_wr_write_chirp_header_cb
        );
    else if(msg->data_len > 0)
        ch_cn_write(
            conn,
            msg->data,
            msg->data_len,
            _ch_wr_write_data_cb
        );
    else
        _ch_wr_write_finish(chirp, writer, conn);
}

// .. c:function::
static
void
_ch_wr_write_timeout_cb(uv_timer_t* handle)
//    :noindex:
//
//    see: :c:func:`_ch_wr_write_timeout_cb`
//
// .. code-block:: cpp
//
{
    ch_connection_t* conn = handle->data;
    ch_writer_t* writer = &conn->writer;
    ch_chirp_t* chirp = conn->chirp;
    A(chirp->_init == CH_CHIRP_MAGIC, "Not a ch_chirp_t*");
    LC(
        chirp,
        "Write timed out. ", "ch_connection_t:%p",
        (void*) conn
    );
    ch_cn_shutdown(conn, CH_TIMEOUT);
    uv_timer_stop(&writer->send_timeout);
}

// .. c:function::
CH_EXPORT
int
ch_chirp_send(ch_chirp_t* chirp, ch_message_t* msg, ch_send_cb_t send_cb)
//    :noindex:
//
//    see: :c:func:`ch_wr_send`
//
// .. code-block:: cpp
//
{
    A(chirp->_init == CH_CHIRP_MAGIC, "Not a ch_chirp_t*");
    if(chirp->_->config.ACKNOWLEDGE != 0)
        msg->type     = CH_MSG_REQ_ACK;
    return ch_wr_send(chirp, msg, send_cb);
}

// .. c:function::
CH_EXPORT
int
ch_chirp_send_ts(ch_chirp_t* chirp, ch_message_t* msg, ch_send_cb_t send_cb)
//    :noindex:
//
//    see: :c:func:`ch_chirp_send_ts`
//
// .. code-block:: cpp
//
{
    A(chirp->_init == CH_CHIRP_MAGIC, "Not a ch_chirp_t*");
    ch_chirp_int_t* ichirp = chirp->_;
    uv_mutex_lock(&ichirp->send_ts_queue_lock);
    if(msg->_flags & CH_MSG_USED) {
        EC(
            chirp,
            "Message already used. ", "ch_message_t:%p",
            (void*) msg
        );
        return CH_USED;
    }
    msg->_send_cb = send_cb;
    ch_msg_enqueue(&ichirp->send_ts_queue, msg);
    uv_mutex_unlock(&ichirp->send_ts_queue_lock);
    uv_async_send(&ichirp->send_ts);
    return CH_SUCCESS;
}

// .. c:function::
void
ch_wr_free(ch_writer_t* writer)
//    :noindex:
//
//    see: :c:func:`ch_wr_free`
//
// .. code-block:: cpp
//
{
    ch_connection_t* conn = writer->send_timeout.data;
    ch_chirp_t* chirp = conn->chirp;
    A(chirp->_init == CH_CHIRP_MAGIC, "Not a ch_chirp_t*");
    uv_close((uv_handle_t*) &writer->send_timeout, ch_cn_close_cb);
    conn->shutdown_tasks += 1;
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
    tmp_err = uv_timer_init(ichirp->loop, &writer->send_timeout);
    if(tmp_err != CH_SUCCESS) {
        E(
            chirp,
            "Initializing send timeout failed: %d",
            tmp_err
        );
    }
    writer->send_timeout.data = conn;
}

// .. c:function::
int
ch_wr_process_queues(ch_remote_t* remote)
//    :noindex:
//
//    see: :c:func:`ch_wr_process_queues`
//
// .. code-block:: cpp
//
{
    ch_chirp_t* chirp = remote->chirp;
    A(chirp->_init == CH_CHIRP_MAGIC, "Not a ch_chirp_t*");
    ch_connection_t* conn = remote->conn;
    A(conn != NULL, "The connection must be set");
    ch_message_t* msg = NULL;
    if(conn->writer.msg != NULL)
        return CH_BUSY;
    if(remote->flags & CH_RM_RETRY_WAITING_MSG) {
        A(
            remote->wait_ack_message,
            "When retrying the wait_ack_message should be set"
        )
        remote->flags  &= ~CH_RM_RETRY_WAITING_MSG;
        ch_wr_write(conn, remote->wait_ack_message);
        return CH_SUCCESS;
    } else if(remote->no_rack_msg_queue != NULL) {
        ch_msg_dequeue(&remote->no_rack_msg_queue, &msg);
        ch_wr_write(conn, msg);
        return CH_SUCCESS;
    } else if(remote->rack_msg_queue != NULL) {
        if(remote->wait_ack_message == NULL) {
            ch_msg_dequeue(&remote->rack_msg_queue, &msg);
            remote->wait_ack_message = msg;
            ch_wr_write(conn, msg);
            return CH_SUCCESS;
        } else
            return CH_BUSY;
    }
    return CH_EMPTY;
}

// .. c:function::
int
ch_wr_send(ch_chirp_t* chirp, ch_message_t* msg, ch_send_cb_t send_cb)
//    :noindex:
//
//    see: :c:func:`ch_wr_send`
//
// .. code-block:: cpp
//
{
    A(chirp->_init == CH_CHIRP_MAGIC, "Not a ch_chirp_t*");
    ch_chirp_int_t* ichirp  = chirp->_;
    if(ichirp->flags & CH_CHIRP_CLOSING || ichirp->flags & CH_CHIRP_CLOSED) {
        if(send_cb != NULL)
            send_cb(msg, CH_SHUTDOWN, -1);
        return CH_SHUTDOWN;
    }
    int tmp_err;
    ch_remote_t      search_remote;
    ch_remote_t*     remote;
    ch_connection_t* conn;
    msg->_send_cb = send_cb;
    msg->_flags  |= CH_MSG_USED;
    ch_protocol_t* protocol = &ichirp->protocol;
    ch_random_ints_as_bytes(msg->serial, sizeof(msg->serial));

    ch_rm_init_from_msg(chirp, &search_remote, msg);
    if(ch_rm_find(protocol->remotes, &search_remote, &remote) != 0) {
        remote = ch_alloc(sizeof(ch_remote_t));
        *remote = search_remote;
        tmp_err = ch_rm_insert(&protocol->remotes, remote);
        A(tmp_err == 0, "Inserting remote failed");
    }

    if(msg->type & CH_MSG_REQ_ACK)
        ch_msg_enqueue(&remote->rack_msg_queue, msg);
    else
        ch_msg_enqueue(&remote->no_rack_msg_queue, msg);

    conn = remote->conn;
    if(conn == NULL) {
        conn = ch_alloc(sizeof(ch_connection_t));
        remote->conn = conn;
        if(!conn) {
            E(
                chirp,
                "Could not allocate memory for connection%s", ""
            );
            msg->_flags &= ~CH_MSG_USED;
            if(send_cb != NULL)
                send_cb(msg, CH_ENOMEM, -1);
            return CH_ENOMEM;
        }
        memset(conn, 0, sizeof(ch_connection_t));
        msg->_conn         = conn;
        conn->port         = msg->port;
        conn->ip_protocol  = msg->ip_protocol;
        conn->connect.data = msg;
        conn->remote       = remote;
        ch_text_address_t taddr;
        ch_msg_get_address(msg, &taddr);
        if(!(
                ichirp->config.DISABLE_ENCRYPTION  ||
                ch_is_local_addr(&taddr)
        )) {
            conn->flags |= CH_CN_ENCRYPTED;
        }
        memcpy(
            &conn->address,
            &msg->address,
            CH_IP_ADDR_SIZE
        );
        uv_tcp_init(ichirp->loop, &conn->client);
        if(msg->ip_protocol == CH_IPV6) {
            struct sockaddr_in6 addr;
            uv_ip6_addr(taddr.data, msg->port, &addr);
            tmp_err = uv_tcp_connect(
                &conn->connect,
                &conn->client,
                (struct sockaddr*) &addr,
                _ch_wr_connect_cb
            );
        } else {
            A(msg->ip_protocol == CH_IPV4, "Unknown IP protocol");
            struct sockaddr_in addr;
            uv_ip4_addr(taddr.data, msg->port, &addr);
            tmp_err = uv_tcp_connect(
                &conn->connect,
                &conn->client,
                (struct sockaddr*) &addr,
                _ch_wr_connect_cb
            );
        }
        LC(
            chirp,
            "Connecting to remote %s:%d. ", "ch_connection_t:%p",
            taddr.data,
            msg->port,
            (void*) conn
        );
        if(tmp_err != CH_SUCCESS) {
            E(
                chirp,
                "Failed to connect to host: %s:%d (%d)",
                taddr.data,
                msg->port,
                tmp_err
            );
        }
    } else
        ch_wr_process_queues(remote);
    return CH_SUCCESS;
}


// .. c:function::
void
ch_wr_write(ch_connection_t* conn, ch_message_t* msg)
//    :noindex:
//
//    see: :c:func:`ch_wr_write`
//
// .. code-block:: cpp
//
{
    ch_chirp_t* chirp = conn->chirp;
    A(chirp->_init == CH_CHIRP_MAGIC, "Not a ch_chirp_t*");
    ch_writer_t* writer = &conn->writer;
    ch_chirp_int_t* ichirp = chirp->_;
    msg->_conn = conn;
    A(writer->msg == NULL, "Message should be null on new write");
    writer->msg = msg;
    int tmp_err = uv_timer_start(
        &writer->send_timeout,
        _ch_wr_write_timeout_cb,
        ichirp->config.TIMEOUT * 1000,
        0
    );
    if(tmp_err != CH_SUCCESS) {
        EC(
            chirp,
            "Starting send timeout failed: %d. ", "ch_connection_t:%p",
            tmp_err,
            (void*) conn
        );
    }

// Use the writers net message structure to write the actual message over
// the connection. The net message structure is of type
// :c:type:`ch_msg_message_t`, which is actually :c:macro:`CH_WIRE_MESSAGE`.
// The difference between ``msg`` and ``net_msg`` is, that ``msg`` is of
// type :c:type:`ch_message_t` and ``net_msg`` of type
// :c:macro:`CH_WIRE_MESSAGE`. That means ``net_msg`` is stripped down to
// essentially only the identity, the serial number, the message type and
// the lengths of the header and the data.
//
// .. code-block:: cpp
//
    ch_msg_message_t* net_msg = &writer->net_msg;
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
    net_msg->type         = msg->type;
    net_msg->header_len   = htons(msg->header_len);
    net_msg->data_len     = htonl(msg->data_len);
    ch_cn_write(
        conn,
        net_msg,
        sizeof(ch_msg_message_t),
        _ch_wr_write_msg_header_cb
    );
}
