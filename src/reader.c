// ======
// Reader
// ======
//
// .. code-block:: cpp
//
#include "reader.h"

#include "chirp.h"
#include "connection.h"
#include "util.h"

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
//    Handle handshake
//
//    :param ch_connection_t* conn: Connection
//    :param ch_readert* reader: Reader
//    :param ch_buf* buf: Buffer containing bytes read
//    :param size_t read: Count of bytes read
//
// .. c:function::
static
ch_inline
int
_ch_rd_read_buffer(
        ch_connection_t* conn,
        ch_reader_t* reader,
        ch_buf* source_buf,
        size_t read,
        size_t *bytes_handled,
        ch_rd_state_t state

);
//
//    Handle handshake
//
//    :param ch_connection_t* conn: Connection
//    :param ch_readert* reader: Reader
//    :param ch_buf* buf: Buffer containing bytes read
//    :param size_t read: Count of bytes read
//    :param ch_rd_state_t state: Current state of the reader
//
//
// Definitions
// ===========

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
    struct sockaddr_storage addr;
    int addr_len;
    ch_connection_t* old_conn;
    ch_chirp_t* chirp = conn->chirp;
    A(chirp->_init == CH_CHIRP_MAGIC, "Not a ch_chirp_t*");
    ch_chirp_int_t* ichirp = chirp->_;
    ch_protocol_t* protocol = &ichirp->protocol;
    if(read < sizeof(ch_rd_handshake_t)) {
        E(
            chirp,
            "Illegal handshake size -> shutdown. ch_chirp_t:%p, "
            "ch_connection_t:%p",
            (void*) chirp,
            (void*) conn
        );
        ch_cn_shutdown(conn);
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
    if(uv_tcp_getpeername(
                &conn->client,
                (struct sockaddr*) &addr,
                &addr_len
    ) != CH_SUCCESS) {
        E(
            chirp,
            "Could not get remote address. ch_chirp_t:%p, "
            "ch_connection_t:%p",
            (void*) chirp,
            (void*) conn
        );
        ch_cn_shutdown(conn);
        return;
    };
    if(addr.ss_family == AF_INET6) {
        struct sockaddr_in6* saddr = (struct sockaddr_in6*) &addr;
        conn->ip_protocol = CH_IPV6;
        memcpy(
            &conn->address,
            &saddr->sin6_addr,
            sizeof(saddr->sin6_addr)
        );
    } else {
        struct sockaddr_in* saddr = (struct sockaddr_in*) &addr;
        conn->ip_protocol = CH_IPV4;
        memcpy(
            &conn->address,
            &saddr->sin_addr,
            sizeof(saddr->sin_addr)
        );
    }
    old_conn = sglib_ch_connection_t_find_member(
        protocol->connections,
        conn
    );
    // If there is a network race condition we replace the old connection and
    // leave the old one for garbage collection
    if(old_conn) {
        // Since we reuse the tree members we have to delete the connection
        // from the old data-structure, before adding it to the new.
        L(
            chirp,
            "ch_connection_t:%p replaced ch_connection_t:%p. "
            "ch_chirp_t:%p",
            (void*) conn,
            (void*) old_conn,
            (void*) chirp
        );
        sglib_ch_connection_t_delete(
            &protocol->connections,
            old_conn
        );
        sglib_ch_connection_set_t_add(
            &protocol->old_connections,
            old_conn
        );
    }
    sglib_ch_connection_t_add(
        &protocol->connections,
        conn
    );
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
        L(
            chirp,
            "Handshake with remote %s:%d (%s) done. ch_chirp_t:%p, "
            "ch_connection_t:%p",
            addr.data,
            conn->port,
            identity,
            (void*) chirp,
            (void*) conn
        );
    }
#   endif
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
    ch_ms_message_t* msg;
    ch_buf* buf = buffer; // Don't do pointer arithmetics on void*
    size_t bytes_handled = 0;
    ch_chirp_t* chirp = conn->chirp;
    A(chirp->_init == CH_CHIRP_MAGIC, "Not a ch_chirp_t*");
    ch_chirp_int_t* ichirp = chirp->_;
    ch_reader_t* reader = &conn->reader;
    do {
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
                    NULL
                );
                reader->state = CH_RD_HANDSHAKE;
                break;
            case CH_RD_HANDSHAKE:
                // We expect that complete handshake arrives at once,
                // check in _ch_rd_handshake
                _ch_rd_handshake(
                    conn,
                    reader,
                    buf + bytes_handled,
                    read - bytes_handled
                );
                bytes_handled += sizeof(ch_rd_handshake_t);
                reader->state = CH_RD_WAIT;
                break;
            case CH_RD_WAIT:
                // We expect that complete message header arrives at once
                if(read + bytes_handled < sizeof(ch_ms_message_t)) {
                    E(
                        chirp,
                        "Illegal message header size -> shutdown. "
                        "ch_chirp_t:%p, ch_connection_t:%p",
                        (void*) chirp,
                        (void*) conn
                    );
                    ch_cn_shutdown(conn);
                    return;
                }
                msg = &reader->msg;
                memcpy(
                    msg,
                    buf + bytes_handled,
                    sizeof(ch_ms_message_t)
                );
                msg->header_len    = ntohs(msg->header_len);
                msg->actor_len     = ntohs(msg->actor_len);
                msg->data_len      = ntohl(msg->data_len);
                bytes_handled     += sizeof(ch_ms_message_t);
                reader->bytes_read = 0; // Reset partial buffer reads
                // Direct jump to next read state
                if(msg->header_len > 0)
                    reader->state = CH_RD_HEADER;
                else if(msg->actor_len > 0)
                    reader->state = CH_RD_ACTOR;
                else if(msg->data_len > 0)
                    reader->state = CH_RD_DATA;
                else {
                    reader->state = CH_RD_WAIT;
                    //TODO ack
                }
                break;
            case CH_RD_HEADER:
                msg = &reader->msg;
                if(_ch_rd_read_buffer(
                    conn,
                    reader,
                    buf + bytes_handled,
                    read - bytes_handled,
                    &bytes_handled,
                    CH_RD_HEADER
                )) break;
                reader->bytes_read = 0; // Reset partial buffer reads
                // Direct jump to next read state
                if(msg->actor_len > 0)
                    reader->state = CH_RD_ACTOR;
                else if(msg->data_len > 0)
                    reader->state = CH_RD_DATA;
                else {
                    reader->state = CH_RD_WAIT;
                    //TODO ack
                }
                break;
            case CH_RD_ACTOR:
                msg = &reader->msg;
                if(_ch_rd_read_buffer(
                    conn,
                    reader,
                    buf + bytes_handled,
                    read - bytes_handled,
                    &bytes_handled,
                    CH_RD_HEADER
                )) break;
                reader->bytes_read = 0; // Reset partial buffer reads
                // Direct jump to next read state
                if(msg->data_len > 0)
                    reader->state = CH_RD_DATA;
                else {
                    reader->state = CH_RD_WAIT;
                    //TODO ack
                }
                break;
            case CH_RD_DATA:
                msg = &reader->msg;
                if(_ch_rd_read_buffer(
                    conn,
                    reader,
                    buf + bytes_handled,
                    read - bytes_handled,
                    &bytes_handled,
                    CH_RD_HEADER
                )) break;
                reader->bytes_read = 0; // Reset partial buffer reads
                reader->state = CH_RD_WAIT;
                //TODO ack
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
        ch_reader_t* reader,
        ch_buf* source_buf,
        size_t read,
        size_t *bytes_handled,
        ch_rd_state_t state

)
//    :noindex:
//
//    see: :c:func:`ch_rd_read`
//
// .. code-block:: cpp
//
{
    (void)(conn);
    (void)(reader);
    (void)(source_buf);
    (void)(read);
    (void)(bytes_handled);
    (void)(state);
    return 1;
}
