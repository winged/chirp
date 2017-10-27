// ======
// Reader
// ======
//

// Project includes
// ================
//
// .. code-block:: cpp
//
#include "chirp.h"
#include "connection.h"
#include "reader.h"
#include "writer.h"
#include "util.h"
#include "remote.h"

// Declarations
// ============

// .. c:function::
static
void
_ch_rd_handshake(
        ch_connection_t* conn,
        ch_buf* buf,
        size_t read
);
//
//    Handle a handshake on the given connection.
//
//    Ensures, that the byte count which shall be read is at least the same as
//    the handshike size.
//
//    The given buffer gets copied into the handshake structure of the reader.
//    Then the port, the maximum time until a timeout happens and the remote
//    identity are applied to the given connection coming from the readers
//    handshake.
//
//    It is then ensured, that the address of the peer connected to the TCP
//    handle (TCP stream) of the connections client can be resolved.
//
//    If the latter was successful, the IP protocol and address are then applied
//    from the resolved address structure to the connection.
//
//    The given connection gets then searched on the protocols pool of
//    connections. If the connection is already known, the found duplicate gets
//    removed from the pool of connections and gets then added to another pool
//    (of the protocol), holding only such old connections.
//
//    Finally, the given connection is added to the protocols pool of
//    connections.
//
//    :param ch_connection_t* conn: Pointer to a connection instance.
//    :param ch_readert* reader:    Pointer to a reader instance. Its handshake
//                                  data structure is the target of this
//                                  handshake
//    :param ch_buf* buf:           Buffer containing bytes read, acts as data
//                                  source
//    :param size_t read:           Count of bytes read
//
// .. c:function::
static
void
_ch_rd_handshake_cb(uv_write_t* req, int status);
//
//    Called when handshake is sent.
//
//    :param uv_write_t* req: Write request type, holding the
//                            connection handle
//    :param int status: Send status
//

// .. c:function::
static
void
_ch_rd_handle_msg(
        ch_connection_t* conn,
        ch_reader_t*     reader,
        ch_message_t*    msg
);
//
//    Send ack and call message-handler
//
//    :param ch_connection_t* conn:  Pointer to a connection instance.
//    :param ch_reader_t* reader:    Pointer to a reader instance.
//    :param ch_message_t* msg:      Message that was received
//

// .. c:function::
static
int
_ch_rd_read_buffer(
        ch_connection_t* conn,
        ch_reader_t*     reader,
        ch_message_t*    msg,
        ch_buf*          src_buf,
        size_t           to_read,
        char**           assign_buf,
        ch_buf*          dest_buf,
        size_t           dest_buf_size,
        uint32_t         expected,
        int              free_flag,
        size_t*          bytes_handled
);
//
//    Read data from the read buffer provided by libuv ``src_buf`` to the field
//    of the message ``assign_buf``. A preallocated buffer will be used if the
//    message is small enough, otherwise a buffer will be allocated. The
//    function also handles partial reads.
//
// Definitions
// ===========

// .. c:type:: _ch_rd_state_names
//
//    Names of reader states for logging.
//
// .. code-block:: cpp
//
char* _ch_rd_state_names[] = {
    "CH_RD_START",
    "CH_RD_HANDSHAKE",
    "CH_RD_WAIT",
    "CH_RD_HEADER",
    "CH_RD_DATA",
};

// .. c:macro:: ch_rd_log_ip_and_id_to_string_m
//
//    Converts an ip and an id to string.
//
//    :param identity: The identity to convert
//
// .. code-block:: cpp

#begindef ch_rd_log_ip_and_id_to_string_m(identity)
    ch_text_address_t addr;
    char id[CH_ID_SIZE * 2 + 1];
    uv_inet_ntop(
        conn->ip_protocol,
        conn->address,
        addr.data,
        sizeof(addr)
    );
    ch_bytes_to_hex(
        identity,
        sizeof(identity),
        id,
        sizeof(id)
    );
#enddef

// .. c:function::
static
void
_ch_rd_handshake(
        ch_connection_t* conn,
        ch_buf* buf,
        size_t read
)
//    :noindex:
//
//    see: :c:func:`_ch_rd_handshake`
//
// .. code-block:: cpp
//
{
    ch_connection_t* old_conn = NULL;
    ch_chirp_t* chirp = conn->chirp;
    ch_remote_t  search_remote;
    ch_remote_t* remote = NULL;
    ch_chirp_int_t* ichirp = chirp->_;
    ch_protocol_t* protocol = &ichirp->protocol;
    ch_sr_handshake_t hs_tmp;
    if(read < CH_SR_HANDSHAKE_SIZE) {
        EC(
            chirp,
            "Illegal handshake size -> shutdown. ", "ch_connection_t:%p",
            (void*) conn
        );
        ch_cn_shutdown(conn, CH_PROTOCOL_ERROR);
        return;
    }
    ch_sr_buf_to_hs(buf, &hs_tmp);
    conn->port = hs_tmp.port;
    conn->max_timeout = hs_tmp.max_timeout;
    memcpy(
        conn->remote_identity,
        hs_tmp.identity,
        CH_ID_SIZE
    );
    ch_rm_init_from_conn(chirp, &search_remote, conn);
    if(ch_rm_find(protocol->remotes, &search_remote, &remote) != 0) {
        remote = ch_alloc(sizeof(*remote));
        *remote = search_remote;
        int tmp_err = ch_rm_insert(&protocol->remotes, remote);
        A(tmp_err == 0, "Inserting remote failed");
    }
    conn->remote = remote;
    /* If there is a network race condition we replace the old connection and
     * leave the old one for garbage collection */
    old_conn = remote->conn;
    remote->conn = conn;
    if(old_conn != NULL) {
        /* If we found the current connection everything is ok */
        if(conn != old_conn) {
            L(
                chirp,
                "ch_connection_t:%p replaced ch_connection_t:%p",
                (void*) conn,
                (void*) old_conn
            );
            ch_cn_old_push(
                &protocol->old_connections,
                old_conn
            );
        }
    }
#   ifndef NDEBUG
    {
        ch_rd_log_ip_and_id_to_string_m(conn->remote_identity);
        LC(
            chirp,
            "Handshake with remote %s:%d (%s) done. ", "ch_connection_t:%p",
            addr.data,
            conn->port,
            id,
            (void*) conn
        );
    }
#   endif
    A(conn->remote != NULL, "The remote has to be set");
    ch_wr_process_queues(conn->remote);
}

// .. c:function::
static
void
_ch_rd_handle_msg(
        ch_connection_t* conn,
        ch_reader_t*     reader,
        ch_message_t*    msg
)
//    :noindex:
//
//    see: :c:func:`_ch_rd_handle_msg`
//
// .. code-block:: cpp
//
{
    ch_chirp_t* chirp = conn->chirp;
    ch_chirp_int_t* ichirp = chirp->_;
#   ifndef NDEBUG
    {
        ch_rd_log_ip_and_id_to_string_m(msg->identity);
        LC(
            chirp,
            "Read message with id: %s\n"
            "                          "
            "serial:%u\n"
            "                          "
            "from %s:%d type:%d data_len:%u. ", "ch_connection_t:%p",
            id,
            msg->serial,
            addr.data,
            conn->port,
            msg->type,
            msg->data_len,
            (void*) conn
        );
    }
#   endif

    reader->state = CH_RD_WAIT;
    reader->handler = NULL;

    if(msg->type & CH_MSG_REQ_ACK) {
        /* Send ack */
        ch_message_t* ack_msg = &reader->ack_msg;
        memset(ack_msg, 0, sizeof(*ack_msg));
        memcpy(ack_msg->identity, msg->identity, CH_ID_SIZE);
        memcpy(ack_msg->address, msg->address, CH_IP_ADDR_SIZE);
        ack_msg->ip_protocol = msg->ip_protocol;
        ack_msg->type        = CH_MSG_ACK;
        ack_msg->header_len  = 0;
        ack_msg->data_len    = 0;
        ack_msg->port        = msg->port;
        ch_wr_send(chirp, ack_msg, NULL);
    } else if(msg->type & CH_MSG_ACK) {
        ch_message_t* wam = conn->remote->wait_ack_message;
        if(memcmp(
                wam->identity,
                msg->identity,
                CH_ID_SIZE
        ) == 0) {
            wam->_flags |= CH_MSG_ACK_RECEIVED;
            conn->remote->wait_ack_message = NULL;
            ch_chirp_try_message_finish(
                chirp,
                conn,
                wam,
                CH_SUCCESS,
                conn->load
            );
        } else {
            EC(
                chirp,
                "Received bad ack -> shutdown. ",
                "ch_connection_t:%p",
                (void*) conn
            );
            /* shutting down because of bad ack is probably overreacted, but we
             * keep it for the moment */
            ch_cn_shutdown(conn, CH_PROTOCOL_ERROR);
            return;
        }
    }
    if(!(msg->type & CH_MSG_ACK)) {
        if(ichirp->recv_cb != NULL) {
            ichirp->recv_cb(chirp, msg);
        } else {
            E(
                chirp,
                "No receiving callback function registered%s", ""
            );
            ch_chirp_release_recv_handler(msg);
        }
    } else
        ch_bf_release(&ichirp->pool, msg->_handler);

}

// .. c:function::
static
void
_ch_rd_handshake_cb(uv_write_t* req, int status)
//    :noindex:
//
//    see: :c:func:`_ch_rd_handshake_cb`
//
// .. code-block:: cpp
//
{
    ch_connection_t* conn = req->data;
    ch_chirp_t* chirp = conn->chirp;
    ch_chirp_check_m(chirp);
    if(status < 0) {
        LC(
            chirp,
            "Sending handshake failed. ", "ch_connection_t:%p",
            (void*) conn
        );
        ch_cn_shutdown(conn, status);
        return;
    }
    /* Check if we already have a message (just after handshake) */
    if(conn->flags & CH_CN_ENCRYPTED)
        ch_pr_read(conn);
}

// .. c:function::
void
ch_rd_read(ch_connection_t* conn, void* buffer, size_t bytes_read)
//    :noindex:
//
//    see: :c:func:`ch_rd_read`
//
// .. code-block:: cpp
//
{
    ch_message_t* msg;
    ch_bf_handler_t* handler;
    ch_buf* buf = buffer; /* Don't do pointer arithmetics on void* */

    /* Bytes handled is used when multiple writes (of the remote) come in a
     * single read and the reader switches between various states as for
     * example CH_RD_HANDSHAKE, CH_RD_WAIT or CH_RD_HEADER. */
    size_t bytes_handled = 0;

    ch_chirp_t* chirp = conn->chirp;
    ch_chirp_check_m(chirp);
    ch_chirp_int_t* ichirp = chirp->_;
    ch_reader_t* reader = &conn->reader;
    LC(
        chirp,
        "Reader state: %s. ", "ch_connection_t:%p",
        _ch_rd_state_names[reader->state],
        (void*) conn
    );
    do {
        int to_read = bytes_read - bytes_handled;

        switch(reader->state) {
            case CH_RD_START:
            {
                ch_sr_handshake_t hs_tmp;
                ch_buf hs_buf[CH_SR_HANDSHAKE_SIZE];
                /* This happens seldom, no copy optimization needed */
                hs_tmp.port = ichirp->public_port;
                hs_tmp.max_timeout = (
                    ichirp->config.RETRIES + 2
                ) * ichirp->config.TIMEOUT;
                memcpy(hs_tmp.identity, ichirp->identity, 16);
                ch_sr_hs_to_buf(&hs_tmp, hs_buf);
                ch_cn_write(
                    conn,
                    hs_buf,
                    CH_SR_HANDSHAKE_SIZE,
                    _ch_rd_handshake_cb
                );
                reader->state = CH_RD_HANDSHAKE;
                break;
            }
            case CH_RD_HANDSHAKE:
                /* We expect that complete handshake arrives at once,
                 * check in _ch_rd_handshake */
                _ch_rd_handshake(
                    conn,
                    buf + bytes_handled,
                    to_read
                );
                bytes_handled += sizeof(ch_sr_handshake_t);
                reader->state = CH_RD_WAIT;
                break;
            case CH_RD_WAIT:
                /* TODO add read timeout (dos protection) */
                if(reader->handler == NULL) {
                    reader->handler = ch_bf_acquire(
                        &ichirp->pool,
                        &reader->last_handler
                    );
                }
                handler = reader->handler;
                msg     = &handler->msg;
                if(to_read >= CH_SR_WIRE_MESSAGE_SIZE) {
                    /* We can read everything */
                    size_t reading = (
                        CH_SR_WIRE_MESSAGE_SIZE - reader->bytes_read
                    );
                    memcpy(
                        reader->net_msg + reader->bytes_read,
                        buf + bytes_handled,
                        reading
                    );
                    reader->bytes_read = 0; /* Reset partial buffer reads */
                    bytes_handled += reading;
                } else {
                    memcpy(
                        reader->net_msg + reader->bytes_read,
                        buf + bytes_handled,
                        to_read
                    );
                    reader->bytes_read += to_read;
                    bytes_handled += to_read;
                    return;
                }
                ch_sr_buf_to_msg(reader->net_msg, msg);
                if((
                        msg->header_len + msg->data_len
                ) > CH_MSG_SIZE_HARDLIMIT)
                {
                    EC(
                        chirp,
                        "Message size exceeds hardlimit. ",
                        "ch_connection_t:%p",
                        (void*) conn
                    );
                    ch_cn_shutdown(conn, CH_ENOMEM);
                    return;
                }
                msg->ip_protocol   = conn->ip_protocol;
                msg->port          = conn->port;
                memcpy(
                    msg->address,
                    conn->address,
                    (
                        msg->ip_protocol == AF_INET6
                    ) ? CH_IP_ADDR_SIZE : CH_IP4_ADDR_SIZE
                );
                /* Direct jump to next read state */
                if(msg->header_len > 0)
                    reader->state = CH_RD_HEADER;
                else if(msg->data_len > 0)
                    reader->state = CH_RD_DATA;
                else
                    _ch_rd_handle_msg(conn, reader, msg);
                break;
            case CH_RD_HEADER:
                handler = reader->handler;
                msg     = &handler->msg;
                if(_ch_rd_read_buffer(
                        conn,
                        reader,
                        msg,
                        buf + bytes_handled,
                        to_read,
                        &msg->header,
                        handler->header,
                        CH_BF_PREALLOC_HEADER,
                        msg->header_len,
                        CH_MSG_FREE_HEADER,
                        &bytes_handled
                ) != CH_SUCCESS) {
                    return;
                }
                /* Direct jump to next read state */
                if(msg->data_len > 0)
                    reader->state = CH_RD_DATA;
                else
                    _ch_rd_handle_msg(conn, reader, msg);
                break;
            case CH_RD_DATA:
                handler = reader->handler;
                msg     = &handler->msg;
                if(_ch_rd_read_buffer(
                        conn,
                        reader,
                        msg,
                        buf + bytes_handled,
                        to_read,
                        &msg->data,
                        handler->data,
                        CH_BF_PREALLOC_DATA,
                        msg->data_len,
                        CH_MSG_FREE_DATA,
                        &bytes_handled
                ) != CH_SUCCESS) {
                    return;
                }
                _ch_rd_handle_msg(conn, reader, msg);
                break;
            default:
                A(0, "Unknown reader state");
                break;
        }
    } while(bytes_handled < bytes_read);
}

CH_EXPORT
void
ch_chirp_release_recv_handler(ch_message_t* msg)
//    :noindex:
//
//    see: :c:func:`ch_chirp_release_recv_handler`
//
// .. code-block:: cpp
//
{
    ch_buffer_pool_t* pool = msg->_pool;
    if(!(msg->_flags & CH_MSG_IS_HANDLER)) {
        fprintf(
            stderr,
            "%s:%d Fatal: Release of non handler message. "
            "ch_buffer_pool_t:%p\n",
            __FILE__,
            __LINE__,
            (void*) pool
        );
        return;
    }
    if(msg->_flags & CH_MSG_FREE_DATA)
        ch_free(msg->data);
    if(msg->_flags & CH_MSG_FREE_HEADER)
        ch_free(msg->header);
    ch_bf_release(pool, msg->_handler);
}

static
int
_ch_rd_read_buffer(
        ch_connection_t* conn,
        ch_reader_t*     reader,
        ch_message_t*    msg,
        ch_buf*          src_buf,
        size_t           to_read,
        char**           assign_buf,
        ch_buf*          dest_buf,
        size_t           dest_buf_size,
        uint32_t         expected,
        int              free_flag,
        size_t*          bytes_handled
)
//    :noindex:
//
//    see: :c:func:`_ch_rd_read_buffer`
//
// .. code-block:: cpp
//
{
    if(reader->bytes_read == 0) {
        if(expected <= dest_buf_size) {
            /* Preallocated buf is large enough */
            *assign_buf = dest_buf;
        } else {
            *assign_buf = ch_alloc(expected);
            if(*assign_buf == NULL) {
                EC(
                    conn->chirp,
                    "Could not allocate memory for message. ",
                    "ch_connection_t:%p",
                    (void*) conn
                );
                ch_cn_shutdown(conn, CH_ENOMEM);
                return CH_ENOMEM;
            }
            msg->_flags |= free_flag;
        }
    }
    if((to_read + reader->bytes_read) >= expected) {
        /* We can read everything */
        size_t reading =  expected - reader->bytes_read;
        memcpy(
            *assign_buf + reader->bytes_read,
            src_buf,
            reading
        );
        *bytes_handled += reading;
        reader->bytes_read = 0; /* Reset partial buffer reads */
        return CH_SUCCESS;
    } else {
        /* Only partial read possible */
        memcpy(
            *assign_buf + reader->bytes_read,
            src_buf,
            to_read
        );
        *bytes_handled += to_read;
        reader->bytes_read += to_read;
        return CH_MORE;
    }
}
