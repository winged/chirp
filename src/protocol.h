// ===============
// Protocol Header
// ===============
//
// Implements the chirp protocol in its children ch_reader_t/ch_writer_t, it
// also handles low-level reading (low-level writing is in connection.h/c)
//
// .. code-block:: cpp

#ifndef ch_protocol_h
#define ch_protocol_h

// Project includes
// ================
//
// .. code-block:: cpp
//
#include "libchirp/chirp.h"
#include "connection.h"
#include "rbtree.h"

// Declarations
// ============

// .. c:type:: ch_protocol_t
//
//    Protocol object.
//
//    .. c:member:: struct sockaddr_in addrv4
//
//       BIND_V4 address converted to a sockaddr_in.
//
//    .. c:member:: struct sockaddr_in addrv6
//
//       BIND_V6 address converted to a sockaddr_in6.
//
//    .. c:member:: uv_tcp_t serverv4
//
//       Reference to the libuv tcp server handle, IPv4.
//
//    .. c:member:: uv_tcp_t serverv6
//
//       Reference to the libuv tcp server handle, IPv6.
//
//    .. c:member:: ch_remote_t* remotes
//
//       Pointer to tree of remotes. They can have a connection.
//
//    .. c:member:: ch_connection_t* old_connections
//
//       Pointer to old connections. This is mainly used when there is a
//       network race condition. The then current connections will be replaced
//       and saved as old connections for garbage collection.
//
//    .. c:member:: ch_chirp_t* chirp
//
//       Pointer to the chirp object. See: :c:type:`ch_chirp_t`.
//
// .. code-block:: cpp
//
struct ch_protocol_s {
    struct sockaddr_in  addrv4;
    struct sockaddr_in6 addrv6;
    uv_tcp_t            serverv4;
    uv_tcp_t            serverv6;
    ch_remote_t*        remotes;
    ch_connection_t*    old_connections;
    ch_chirp_t*         chirp;
};

// .. c:function::
ch_error_t
ch_pr_conn_start(
        ch_chirp_t* chirp,
        ch_connection_t* conn,
        uv_tcp_t* client,
        int accept
);
//
//    Start the given connection
//
//    :param ch_chirp_t* chirp: Chirp object
//    :param ch_connection_t* conn: Connection object
//    :param uv_tcp_t* client: Client to start
//    :param int accept: Is accepted connection
//
//    :return: Void since only called from callbacks.
//

// .. c:function::
void
ch_pr_read(ch_connection_t* conn);
//
//    Reads data over SSL on the given connection. Returns 1 if something was
//    read, 0 otherwise.
//
//    :param ch_connection_t* conn: Pointer to a connection handle.

// .. c:function::
ch_error_t
ch_pr_start(ch_protocol_t* protocol);
//
//    Start the given protocol.
//
//    :param ch_protocol_t* protocol: Protocol which shall be started..
//
//    :return: A chirp error. see: :c:type:`ch_error_t`
//    :rtype:  ch_error_t

// .. c:function::
void
ch_pr_read_data_cb(uv_stream_t* stream, ssize_t nread, const uv_buf_t* buf);
//
//    Callback called from libuv when data was read on a stream.
//    Reads nread bytes on either an encrypted or an unencrypted connection
//    coming from the given stream handle.
//
//    :param uv_stream_t* stream: Pointer to the stream that data was read on.
//    :param ssize_t nread: Number of bytes that were read on the stream.
//    :param uv_buf_t* buf: Pointer to a libuv (data-) buffer. When nread < 0,
//                          the buf parameter might not point to a valid
//                          buffer; in that case buf.len and buf.base are both
//                          set to 0.

// .. c:function::
ch_error_t
ch_pr_stop(ch_protocol_t* protocol);
//
//    Stop the given protocol.
//
//    :param ch_protocol_t* protocol: Protocol which shall be stopped.
//
//    :return: A chirp error. see: :c:type:`ch_error_t`
//    :rtype:  ch_error_t

// .. c:function::
void
ch_pr_init(ch_chirp_t* chirp, ch_protocol_t* protocol);
//
//    Initialize the protocol structure.
//
//    :param ch_chirp_t* chirp: Chirp instance.
//    :param ch_protocol_t* protocol: Protocol to initialize.
//
// .. code-block:: cpp
//
#endif //ch_protocol_h
