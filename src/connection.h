// =================
// Connection header
// =================
//
// .. todo:: Document purpose
//
// .. code-block:: cpp
//
#ifndef ch_connection_h
#define ch_connection_h

// Project includes
// ================
//
// .. code-block:: cpp
//
#include "libchirp/chirp.h"
#include "message.h"
#include "reader.h"
#include "writer.h"

// System includes
// ===============
//
// .. code-block:: cpp
//
#include <openssl/bio.h>
#include <openssl/ssl.h>
#include "sglib.h"

// Sglib Prototypes
// ================
//
// .. code-block:: cpp
//
#define CH_CONNECTION_CMP(x,y) ch_connection_cmp(x, y)

// .. code-block:: cpp
//
SGLIB_DEFINE_RBTREE_PROTOTYPES( // NOCOV
    ch_connection_t,
    left,
    right,
    color_field,
    CH_CONNECTION_CMP
)

// .. code-block:: cpp
//
SGLIB_DEFINE_RBTREE_PROTOTYPES( // NOCOV
    ch_connection_set_t,
    left,
    right,
    color_field,
    SGLIB_NUMERIC_COMPARATOR
)

// Declarations
// ============

// .. c:type:: ch_cn_flags_t
//
//    Represents connection flags.
//
//    .. c:member:: CH_CN_SHUTTING_DOWN
//
//       Indicates that the connection is shutting down.
//
//    .. c:member:: CH_CN_WRITE_PENDING
//
//       Indicates that There is a write pending.
//
//    .. c:member:: CH_CN_TLS_HANDSHAKE
//
//      Indicates that a handshake is running.
//
//    .. c:member:: CH_CN_ENCRYPTED
//
//       Indicates whether the connection is encrypted or not.
//
//    .. c:member:: CH_CN_BUF_WTLS_USED
//
//       Indicates if TLS for writing is used.
//
//    .. c:member:: CH_CN_BUF_RTLS_USED
//
//       Indicates if TLS for reading is used.
//
//    .. c:member:: CH_CN_BUF_UV_USED
//
//       Indicates that the connection buffer is currently used by libuv.
//
// .. code-block:: cpp
//
typedef enum {
    CH_CN_SHUTTING_DOWN  = 1 << 0,
    CH_CN_WRITE_PENDING  = 1 << 1,
    CH_CN_TLS_HANDSHAKE  = 1 << 2,
    CH_CN_ENCRYPTED      = 1 << 3,
    CH_CN_BUF_WTLS_USED  = 1 << 4,
    CH_CN_BUF_RTLS_USED  = 1 << 5,
    CH_CN_BUF_UV_USED    = 1 << 6,
} ch_cn_flags_t;

// .. c:type:: ch_connection_t
//
//    Connection dictionary implemented as red-black tree.
//
//    .. c:member:: uint8_t ip_protocol
//
//       What IP protocol (IPv4 or IPv6) shall be used for connections.
//
//    .. c:member:: uint8_t[16] address
//
//       IPv4/6 address of the sender if the message was received.  IPv4/6
//       address of the recipient if the message is going to be sent.
//
//    .. c:member:: int32_t port
//
//       The port that shall be used for connections.
//
//    .. c:member:: uint8_t[16] remote_identity
//
//       The identity of the remote target. This is used for getting the remote
//       address.
//
//    .. c:member:: float max_timeout
//
//       The maximum amount of time in seconds that making a connection is
//       being tried.
//
//    .. c:member:: uv_tcp_t client
//
//       The TCP handle (TCP stream) of the client, which is used to get the
//       address of the peer connected to the handle.
//
//    .. c:member:: uv_buf* buffer_uv
//
//       Pointer to the libuv (data-) buffer data type.
//
//    .. c:member:: uv_buf* buffer_wtls
//
//       Pointer to the libuv buffer data type for writing data over TLS.
//
//    .. c:member:: uv_buf* buffer_rtls
//
//       Pointer to the libuv buffer data type for reading data over TLS.
//
//    .. c:member:: uv_buf_t buffer_uv_uv
//
//       The actual libuv (data-) buffer (using the buffer_uv data type).
//
//    .. c:member:: uv_buf_t buffer_wtls_uv
//
//       The actual libuv buffer for writing data over TLS (using the
//       buffer_wtls data type).
//
//    .. c:member:: uv_buf_t buffer_any_uv
//
//       Generic libuv (data-) buffer used for writing over a connection. The
//       data type of the buffer must be provided when writing.
//
//    .. c:member:: size_t buffer_size
//
//       The size that shall be used for initializing the libuv (data-)
//       buffers.
//
//    .. c:member:: uv_write_cb write_callback
//
//       A callback which will be called after a sucessful write over a
//       connection.
//
//    .. c:member:: size_t write_written
//
//       Holds how many bytes have been written over a connection. This is
//       typically zero at first and gets increased with each partial write.
//
//    .. c:member:: size_t write_size
//
//       Indicates how many bytes shall in total be written over a connection.
//
//    .. c:member:: ch_buf* write_buffer
//
//       Pointer to the write buffer. The buffer that will be written to.
//
//    .. c:member:: ch_chirp_t* chirp
//
//       Pointer to the chirp object. See: :c:type:`ch_chirp_t`.
//
//    .. c:member:: uv_shutdown_t shutdown_req
//
//       Shutdown request object of type uv_shutdown_t. It is used to shutdown
//       the outgoing (write) side of a duplex stream.
//
//    .. c:member:: uv_write_t write_req
//
//       Write request objet, which is used to write data on a handle.
//
//    .. c:member:: uv_timer_t shutdown_timeout
//
//       Timer handle used when shutting down a connection. This is used to set
//       timer which will fire a set callback on the next event loop iteration.
//       The time is specified in milliseconds and provided by
//       :c:type:`ch_config_t`.
//
//    .. c:member:: int8_t shutdown_tasks
//
//       Counter for tasks that need to be done when shutting down a connection.
//       Tasks may be calling closing-callbacks for example, on a request handle
//       or the shutdown timer handle. This acts as semaphore.
//
//    .. c:member:: uint8_t flags
//
//       Flags indicating the state of a connection, e.g. shutting down, write
//       pending, TLS handshake, whether the connection is encrypted or not and
//       so on, see :c:type:`ch_cn_flags_t`.
//
//    .. c:member:: SSL* ssl
//
//       Pointer to a SSL (data-) structure. This is used when using an
//       encrypted connection over SSL.
//
//    .. c:member:: BIO* bio_ssl
//
//       Pointer to the BIO structure of SSL. BIO is an I/O stream abstraction
//       and essentially OpenSSL's answer to the C library's FILE pointer. This
//       is used to create a connected BIO pair alongside with
//       :c:member:`bio_app` and is only used for the SSL connection.
//
//    .. c:member:: BIO* bio_app
//
//       Pointer to the applications BIO structure. This is used to read and
//       write (partial) data over TLS.
//
//    .. c:member:: int tls_handshake_state
//
//       Holds the current state of the SSL handshake when using an encrypted
//       connection and TLS handshakes. This is used within the protocol, see
//       :c:func:`_ch_pr_do_handshake`.
//
//    .. c:member:: float load
//
//       The load of the remote peer. This is used when a protocol error or an
//       timeout happens when writing. See :c:member:`ch_send_cb_t.load`.
//
//    .. c:member:: ch_reader_t reader
//
//       Handle to a chirp reader, handles handshakes and reads (buffers) on a
//       connection.
//
//    .. c:member:: ch_writer_t writer
//
//       Handle to a chirp writer, handles sending and writing on a connection.
//
//    .. c:member:: char color_field
//
//       The color of the current (connection-) node. This may either be red or
//       black, as connections are built as a red-black tree.
//
//    .. c:member:: struct ch_connection_s* left
//
//       (Struct-) Pointer to the left child of the current connection (node)
//       in the red-black tree.
//
//    .. c:member:: struct ch_connection_s* right
//
//       (Struct-) Pointer to the right child of the current connection (node)
//       in the red-black tree.
//
// .. code-block:: cpp
//
typedef struct ch_connection_s {
    uint8_t                 ip_protocol;
    uint8_t                 address[16];
    int32_t                 port;
    uint8_t                 remote_identity[16];
    float                   max_timeout;
    uv_tcp_t                client;
    ch_buf*                 buffer_uv;
    ch_buf*                 buffer_wtls;
    ch_buf*                 buffer_rtls;
    uv_buf_t                buffer_uv_uv;
    uv_buf_t                buffer_wtls_uv;
    uv_buf_t                buffer_any_uv;
    size_t                  buffer_size;
    uv_write_cb             write_callback;
    size_t                  write_written;
    size_t                  write_size;
    ch_buf*                 write_buffer;
    ch_chirp_t*             chirp;
    uv_shutdown_t           shutdown_req;
    uv_write_t              write_req;
    uv_timer_t              shutdown_timeout;
    int8_t                  shutdown_tasks;
    uint8_t                 flags;
    SSL*                    ssl;
    BIO*                    bio_ssl;
    BIO*                    bio_app;
    int                     tls_handshake_state;
    float                   load;
    ch_reader_t             reader;
    ch_writer_t             writer;
    char                    color_field;
    struct ch_connection_s* left;
    struct ch_connection_s* right;
} ch_connection_t;

typedef ch_connection_t ch_connection_set_t;

// .. c:function::
void
ch_cn_close_cb(uv_handle_t* handle);
//
//    Called by libuv after closing a connection handle.
//
//    :param uv_handle_t* handle: The libuv handle holding the
//                                connection

// .. c:function::
void
ch_cn_read_alloc_cb(uv_handle_t* handle, size_t suggested_size, uv_buf_t* buf);
//
//    Allocates buffers on the connection and reuses it for each subsequent
//    reads. Also allocates the buffer for TLS since it has to be the same
//    size.
//
//    :param uv_handle_t* handle: The libuv handle holding the
//                                connection
//    :param size_t suggested_size: The size of the connection buffer
//                                  in bytes.
//    :param uv_buf_t* buf: Libuv buffer which will hold the
//                          connection

// .. c:function::
ch_error_t
ch_cn_shutdown(ch_connection_t* conn);
//
//    Shutdown this connection.
//
//    :param ch_connection_t* conn: Connection dictionary holding a
//                                  chirp instance.
//    :return: A chirp error. see: :c:type:`ch_error_t`
//    :rtype: ch_error_t

// .. c:function::
ch_error_t
ch_cn_init(ch_chirp_t* chirp, ch_connection_t* conn, uint8_t flags);
//
//    Initialize a connection.
//
//    :param ch_chirp_t* chirp: Chirp instance
//    :param ch_connection_t* conn: Connection to initialize
//    :param uint8_t flags: Pass CH_CN_ENCRYPTED for a encrypted connection, 0
//                          otherwise
//

// .. c:function::
ch_error_t
ch_cn_init_enc(ch_chirp_t* chirp, ch_connection_t* conn);
//
//    Initialize encryption.
//
//    :param ch_chirp_t* chirp: Chirp instance
//    :param ch_connection_t* conn: Connection to initialize
//

// .. c:function::
void
ch_cn_send_if_pending(ch_connection_t* conn);
//
//    Send all pending handshake data from SSL
//
//    :param ch_connection_t* conn: Connection
//
//
// .. c:function::
void
ch_cn_write(
        ch_connection_t* conn,
        void* buf,
        size_t size,
        uv_write_cb callback
);
//
//    Send data to remote
//
//    :param ch_connection_t* conn: Connection
//    :param void* buf: Buffer to send. It must stay valid till the callback is
//                      called.
//    :param size_t size: Size of data to send
//    :param uv_write_cb: Callback when data is written, can be NULL
//
//
// .. code-block:: cpp

// Definitions
// ===========

// .. c:function::
static
ch_inline
int
ch_connection_cmp(ch_connection_t* x, ch_connection_t* y)
//
//    Compare operator for connections.
//
//    :param ch_connection_t* x: First connection instance to compare
//    :param ch_connection_t* y: Second connection instance to compare
//
//    :return: the comparision between
//                 - the IP protocols, if they are not the same, or
//                 - the addresses, if they are not the same, or
//                 - the ports
//    :rtype: int
//
// .. code-block:: cpp
//
{
  if(x->ip_protocol != y->ip_protocol) {
    return x->ip_protocol - y->ip_protocol;
  } else {
    int tmp_cmp = memcmp(
                         x->address,
                         y->address,
                         x->ip_protocol == CH_IPV6 ? 16 : 4
                         );
    if(tmp_cmp != 0) {
      return tmp_cmp;
    } else {
      return x->port - y->port;
    }
  }
}

#endif //ch_connection_h
