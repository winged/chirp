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
ch_inline
void
_ch_rd_handshake(
        ch_connection_t* conn,
        ch_reader_t* reader,
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
ch_inline
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
ch_inline
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

// .. c:function::
static
ch_inline
void
_ch_rd_handshake(
        ch_connection_t* conn,
        ch_reader_t* reader,
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
    int tmp_err;
    ch_connection_t* old_conn = NULL;
    ch_chirp_t* chirp = conn->chirp;
    A(chirp->_init == CH_CHIRP_MAGIC, "Not a ch_chirp_t*");
    ch_remote_t  search_remote;
    ch_remote_t* remote = NULL;
    ch_chirp_int_t* ichirp = chirp->_;
    ch_protocol_t* protocol = &ichirp->protocol;
    if(read < sizeof(ch_rd_handshake_t)) {
        EC(
            chirp,
            "Illegal handshake size -> shutdown. ", "ch_connection_t:%p",
            (void*) conn
        );
        ch_cn_shutdown(conn, CH_PROTOCOL_ERROR);
        return;
    }
    memcpy(&reader->hs, buf, sizeof(ch_rd_handshake_t));
    conn->port = ntohs(reader->hs.port);
    conn->max_timeout = ntohs(
        reader->hs.max_timeout + (
            (ichirp->config.RETRIES + 2) * ichirp->config.TIMEOUT
        )
    );
    memcpy(
        conn->remote_identity,
        reader->hs.identity,
        sizeof(conn->remote_identity)
    );
    ch_rm_init_from_conn(chirp, &search_remote, conn);
    if(ch_rm_find(protocol->remotes, &search_remote, &remote) != 0) {
        remote = ch_alloc(sizeof(ch_remote_t));
        *remote = search_remote;
        tmp_err = ch_rm_insert(&protocol->remotes, remote);
        A(tmp_err == 0, "Inserting remote failed");
    }
    conn->remote = remote;
    /* If there is a network race condition we replace the old connection and
     * leave the old one for garbage collection */
    old_conn = remote->conn;
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
        ch_text_address_t addr;
        char identity[33];
        uv_inet_ntop(
            conn->ip_protocol == CH_IPV6 ? AF_INET6 : AF_INET,
            conn->address,
            addr.data,
            sizeof(addr)
        );
        ch_bytes_to_hex(
            conn->remote_identity,
            sizeof(conn->remote_identity),
            identity,
            sizeof(identity)
        );
        LC(
            chirp,
            "Handshake with remote %s:%d (%s) done. ", "ch_connection_t:%p",
            addr.data,
            conn->port,
            identity,
            (void*) conn
        );
    }
#   endif
    A(conn->remote != NULL, "The remote has to be set");
    ch_wr_proccess_queues(conn->remote);
}

// .. c:function::
static
ch_inline
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
    A(chirp->_init == CH_CHIRP_MAGIC, "Not a ch_chirp_t*");
    ch_chirp_int_t* ichirp = chirp->_;
    /* Pause reading on last handler. */
    // TODO start reader again
    if(reader->last) {
        uv_read_stop((uv_stream_t*) &conn->client);
    }
    reader->state = CH_RD_WAIT;

    if(msg->type & CH_MSG_REQ_ACK) {
        /* Send ack */
        ch_message_t* ack_msg = &reader->ack_msg;
        memset(ack_msg, 0, sizeof(ch_message_t));
        memcpy(
            ack_msg,
            msg,
            sizeof(ch_msg_message_t)
        );
        memcpy(
            ack_msg->address,
            msg->address,
            CH_IP_ADDR_SIZE
        );
        ack_msg->type       = CH_MSG_ACK;
        ack_msg->header_len = 0;
        ack_msg->data_len   = 0;
        ack_msg->chirp      = chirp;
        ack_msg->port       = msg->port;
        ack_msg->_conn      = msg->_conn;
        ch_random_ints_as_bytes(
            ack_msg->serial,
            sizeof(ack_msg->serial)
        );
        ch_wr_send(chirp, ack_msg, NULL);
    } else if(msg->type & CH_MSG_ACK) {
        ch_writer_t* writer = &conn->writer;
        if(memcmp(
                writer->msg->identity,
                msg->identity,
                CH_ID_SIZE
        ) == 0) {
            ch_message_t* wmsg = writer->msg;
            writer->flags |= CH_WR_ACK_RECEIVED;
            ch_chirp_try_message_finish(
                chirp,
                writer,
                wmsg,
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
        }
    }

#   ifndef NDEBUG
        ch_text_address_t addr;
        char id[33];
        char serial[33];
        uv_inet_ntop(
            conn->ip_protocol == CH_IPV6 ? AF_INET6 : AF_INET,
            conn->address,
            addr.data,
            sizeof(addr)
        );
        ch_bytes_to_hex(
            msg->identity,
            sizeof(msg->identity),
            id,
            sizeof(id)
        );
        ch_bytes_to_hex(
            msg->serial,
            sizeof(msg->serial),
            serial,
            sizeof(serial)
        );
        LC(
            chirp,
            "Read message with id: %s, serial:%s\n"
            "                          "
            "from %s:%d type:%d data_len:%d. ", "ch_connection_t:%p",
            id,
            serial,
            addr.data,
            conn->port,
            msg->type,
            msg->data_len,
            (void*) conn
        );
#   endif

    /* TODO release to done */
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
    A(chirp->_init == CH_CHIRP_MAGIC, "Not a ch_chirp_t*");
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
ch_rd_read(ch_connection_t* conn, void* buffer, size_t read)
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

    /* Bytes handled is used for the case when multiple data streams are
     * coming in and the reader switches between various states as for
     * example CH_RD_HANDSHAKE, CH_RD_WAIT or CH_RD_HEADER. */
    size_t bytes_handled = 0;

    ch_chirp_t* chirp = conn->chirp;
    A(chirp->_init == CH_CHIRP_MAGIC, "Not a ch_chirp_t*");
    ch_chirp_int_t* ichirp = chirp->_;
    ch_reader_t* reader = &conn->reader;
    LC(
        chirp,
        "Reader state: %s. ", "ch_connection_t:%p",
        _ch_rd_state_names[reader->state],
        (void*) conn
    );
    do {
        int to_read = read - bytes_handled;
        switch(reader->state) {
            case CH_RD_START:
                reader->hs.port = htons(ichirp->public_port);
                reader->hs.max_timeout = htons(
                    (ichirp->config.RETRIES + 2) * ichirp->config.TIMEOUT
                );
                memcpy(reader->hs.identity, ichirp->identity, 16);
                ch_cn_write(
                    conn,
                    &reader->hs,
                    sizeof(ch_rd_handshake_t),
                    _ch_rd_handshake_cb
                );
                reader->state = CH_RD_HANDSHAKE;
                break;
            case CH_RD_HANDSHAKE:
                /* We expect that complete handshake arrives at once,
                 * check in _ch_rd_handshake */
                _ch_rd_handshake(
                    conn,
                    reader,
                    buf + bytes_handled,
                    to_read
                );
                bytes_handled += sizeof(ch_rd_handshake_t);
                reader->state = CH_RD_WAIT;
                break;
            case CH_RD_WAIT:
                /* TODO add read timeout (dos protection) */
                if(reader->bytes_read == 0) {
                    reader->handler = ch_bf_acquire(
                        &ichirp->pool,
                        &reader->last
                    );
                }
                handler = reader->handler;
                msg     = &handler->msg;
                if(to_read >= (int) sizeof(ch_msg_message_t)) {
                    /* We can read everything */
                    memcpy(
                        msg + reader->bytes_read,
                        buf + bytes_handled,
                        sizeof(ch_msg_message_t)
                    );
                } else {
                    memcpy(
                        msg + reader->bytes_read,
                        buf + bytes_handled,
                        to_read
                    );
                    reader->bytes_read += to_read;
                    return;
                }
                msg->header_len    = ntohs(msg->header_len);
                msg->data_len      = ntohl(msg->data_len);
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
                        msg->ip_protocol == CH_IPV6
                    ) ? CH_IP_ADDR_SIZE : CH_IP4_ADDR_SIZE
                );
                bytes_handled     += sizeof(ch_msg_message_t);
                reader->bytes_read = 0; /* Reset partial buffer reads */
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
    } while(bytes_handled < read);
}

// .. c:function::
static
ch_inline
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
        memcpy(
            *assign_buf + reader->bytes_read,
            src_buf,
            expected - reader->bytes_read
        );
    } else {
        /* Only partial read possible */
        memcpy(
            *assign_buf + reader->bytes_read,
            src_buf,
            to_read
        );
        reader->bytes_read += to_read;
        return CH_MORE;
    }
    *bytes_handled += expected;
    reader->bytes_read = 0; /* Reset partial buffer reads */
    return CH_SUCCESS;
}
