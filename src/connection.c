// ==========
// Connection
// ==========
//
// .. todo:: Document purpose
//
// .. code-block:: cpp


// Project includes
// ================
//
// .. code-block:: cpp
//
#include "connection.h"
#include "chirp.h"
#include "util.h"

// System includes
// ===============
//
// .. code-block:: cpp
//
#include <openssl/err.h>

// Sglib Prototypes
// ================

// .. code-block:: cpp
//
SGLIB_DEFINE_RBTREE_FUNCTIONS( // NOCOV
    ch_connection_t,
    left,
    right,
    color_field,
    CH_CONNECTION_CMP
)

// .. code-block:: cpp
//
SGLIB_DEFINE_RBTREE_FUNCTIONS( // NOCOV
    ch_connection_set_t,
    left,
    right,
    color_field,
    SGLIB_NUMERIC_COMPARATOR
)

// Declarations
// ============

// .. c:function::
static
ch_inline
ch_error_t
_ch_cn_allocate_buffers(ch_connection_t* conn);
//
//    Allocate the connections communication buffers.
//
//    :param ch_connection_t* conn: Connection
//


// .. c:function::
static
ch_inline
void
_ch_cn_partial_write(ch_connection_t* conn);
//
//    Called by libuv when pending data has been sent.
//
//    :param ch_connection_t* conn: Connection
//

// .. c:function::
static
void
_ch_cn_send_pending_cb(uv_write_t* req, int status);
//
//    Called during handshake by libuv when pending data has been sent
//
//    :param uv_write_t* req: Write request type, holding the
//                            connection handle
//    :param int status: The status after the shutdown. 0 in case of
//                       success, < 0 otherwise
//

// .. c:function::
static
void
_ch_cn_shutdown_cb(uv_shutdown_t* req, int status);
//
//    Called by libuv after shutting a connection down.
//
//    :param uv_shutdown_t* req: Shutdown request type, holding the
//                               connection handle
//    :param int status: The status after the shutdown. 0 in case of
//                       success, < 0 otherwise

// .. c:function::
static
void
_ch_cn_shutdown_timeout_cb(uv_timer_t* handle);
//
//    Called after shutdown timeout. Closing the connection even though
//    shutdown was delayed.
//
//    :param uv_timer_t* handle: Timer handle to schedule callback

// .. c:function::
static
void
_ch_cn_write_cb(uv_write_t* req, int status);
//
//    Callback used for ch_cn_write.
//
//    :param uv_write_t* req: Write request
//    :param int status: Write status
//

// Definitions
// ===========
//
// .. c:function::
static
ch_inline
ch_error_t
_ch_cn_allocate_buffers(ch_connection_t* conn)
//    :noindex:
//
//    See: :c:func:`_ch_cn_allocate_buffers`
//
// .. code-block:: cpp
//
{
    ch_chirp_t* chirp = conn->chirp;
    A(chirp->_init == CH_CHIRP_MAGIC, "Not a ch_chirp_t*");
    ch_chirp_int_t* ichirp = chirp->_;
    size_t size = ichirp->config.BUFFER_SIZE;
    if(size == 0)
        size = CH_BUFFER_SIZE;
    conn->buffer_uv   = ch_alloc(size);
    if(conn->flags & CH_CN_ENCRYPTED) {
        conn->buffer_wtls  = ch_alloc(size);
        conn->buffer_rtls  = ch_alloc(size);
    }
    conn->buffer_size = size;
    int alloc_nok = 0;
    if(conn->flags & CH_CN_ENCRYPTED)
        alloc_nok = !(
            conn->buffer_uv &&
            conn->buffer_wtls &&
            conn->buffer_rtls
        );
    else
        alloc_nok = !conn->buffer_uv;
    if(alloc_nok) {
        E(
            chirp,
            "Could not allocate memory for libuv and tls. "
            "ch_chirp_t:%p, ch_connection_t:%p",
            (void*) chirp,
            (void*) conn
        );
        return CH_ENOMEM;
    }
    conn->buffer_uv_uv = uv_buf_init(
        conn->buffer_uv,
        conn->buffer_size
    );
    conn->buffer_wtls_uv = uv_buf_init(
        conn->buffer_wtls,
        conn->buffer_size
    );
    return CH_SUCCESS;
}

// .. c:function::
static
ch_inline
void
_ch_cn_partial_write(ch_connection_t* conn)
//    :noindex:
//
//    See: :c:func:`_ch_cn_partial_write`
//
// .. code-block:: cpp
//
{
    size_t bytes_encrypted = 0;
    size_t bytes_read      = 0;
    ch_chirp_t* chirp = conn->chirp;
    A(chirp->_init == CH_CHIRP_MAGIC, "Not a ch_chirp_t*");
    A(!(conn->flags & CH_CN_BUF_WTLS_USED), "The wtls buffer is still used");
    A(!(conn->flags & CH_CN_WRITE_PENDING), "Another uv write is pending");
#   ifndef NDEBUG
        conn->flags |= CH_CN_WRITE_PENDING;
        conn->flags |= CH_CN_BUF_WTLS_USED;
#   endif
    do {
        int tmp_err;
        tmp_err = SSL_write(
            conn->ssl,
            conn->write_buffer + bytes_encrypted + conn->write_written,
            conn->write_size - bytes_encrypted - conn->write_written
        );
        if(tmp_err < 0) {
            E(
                chirp,
                "SSL error writing to BIO, shutting down connection. "
                "ch_connection_t:%p ch_chirp_t:%p",
                (void*) conn,
                (void*) chirp
            );
            ch_cn_shutdown(conn, CH_TLS_ERROR);
            return;
        }
        int read = BIO_read(
            conn->bio_app,
            conn->buffer_wtls + bytes_read,
            conn->buffer_size - bytes_read
        );
        bytes_encrypted += tmp_err;
        bytes_read += read;
    } while(
        ((bytes_encrypted + conn->write_written) < conn->write_size) &&
        (bytes_read < conn->buffer_size)
    );
    conn->buffer_wtls_uv.len = bytes_read;
    uv_write(
        &conn->write_req,
        (uv_stream_t*) &conn->client,
        &conn->buffer_wtls_uv,
        1,
        _ch_cn_write_cb
    );
    L(
        chirp,
        "Called uv_write with %d handshake bytes. "
        "ch_chirp_t:%p, ch_connection_t:%p",
        (int) bytes_read,
        (void*) chirp,
        (void*) conn
    );
    conn->write_written += bytes_encrypted;
}

// .. c:function::
static
void
_ch_cn_send_pending_cb(uv_write_t* req, int status)
//    :noindex:
//
//    see: :c:func:`_ch_cn_send_pending_cb`
//
// .. code-block:: cpp
//
{
    ch_connection_t* conn = req->data;
    ch_chirp_t* chirp = conn->chirp;
    A(chirp->_init == CH_CHIRP_MAGIC, "Not a ch_chirp_t*");
#   ifndef NDEBUG
        conn->flags &= ~CH_CN_WRITE_PENDING;
        conn->flags &= ~CH_CN_BUF_WTLS_USED;
#   endif
    if(status < 0) {
        L(
            chirp,
            "Sending pending data failed. ch_chirp_t:%p, ch_connection_t:%p",
            (void*) chirp,
            (void*) conn
        );
        ch_cn_shutdown(conn, status);
        return;
    }
    L(
        chirp,
        "Write handshake bytes to connection successful. "
        "ch_chirp_t:%p, ch_connection_t:%p",
        (void*) chirp,
        (void*) conn
    );
    ch_cn_send_if_pending(conn);
}

// .. c:function::
static
void
_ch_cn_shutdown_cb(
        uv_shutdown_t* req,
        int status
)
//    :noindex:
//
//    see: :c:func:`_ch_cn_shutdown_cb`
//
// .. code-block:: cpp
//
{
    (void)(status); // Ignore callback-arg
    int tmp_err;
    ch_connection_t* conn = req->handle->data;
    ch_chirp_t* chirp = conn->chirp;
    L(
        chirp,
        "Shutdown callback called. ch_connection_t:%p, ch_chirp_t:%p",
        (void*) conn,
        (void*) chirp
    );
    tmp_err = uv_timer_stop(&conn->shutdown_timeout);
    if(tmp_err != CH_SUCCESS) {
        E(
            chirp,
            "Stopping shutdown timeout failed: %d. ch_connection_t:%p,"
            " ch_chirp_t:%p",
            tmp_err,
            (void*) conn,
            (void*) chirp
        );
    }
    uv_handle_t* handle = (uv_handle_t*) req->handle;
    if(uv_is_closing(handle)) {
        E(
            chirp,
            "Connection already closed after shutdown. "
            "ch_connection_t:%p, ch_chirp_t:%p",
            (void*) conn,
            (void*) chirp
        );
    } else {
        uv_read_stop(req->handle);
        ch_rd_free(&conn->reader);
        ch_wr_free(&conn->writer);
        uv_close(handle, ch_cn_close_cb);
        uv_close((uv_handle_t*) &conn->shutdown_timeout, ch_cn_close_cb);
        conn->shutdown_tasks += 2;
        L(
            chirp,
            "Closing connection after shutdown. "
            "ch_connection_t:%p, ch_chirp_t:%p",
            (void*) conn,
            (void*) chirp
        );
    }
}

// .. c:function::
static
void
_ch_cn_shutdown_timeout_cb( uv_timer_t* handle)
//    :noindex:
//
//    see: :c:func:`_ch_cn_shutdown_timeout_cb`
//
// .. code-block:: cpp
//
{
    ch_connection_t* conn = handle->data;
    int tmp_err;
    ch_chirp_t* chirp = conn->chirp;
    A(chirp->_init == CH_CHIRP_MAGIC, "Not a ch_chirp_t*");
    _ch_cn_shutdown_cb(&conn->shutdown_req, 1);
    tmp_err = uv_cancel((uv_req_t*) &conn->shutdown_req);
    if(tmp_err != CH_SUCCESS) {
        E(
            chirp,
            "Canceling shutdown timeout failed: %d. ch_connection_t:%p,"
            " ch_chirp_t:%p",
            tmp_err,
            (void*) conn,
            (void*) chirp
        );
    }
    L(
        chirp,
        "Shutdown timed out closing. ch_connection_t:%p, ch_chirp_t:%p",
        (void*) conn,
        (void*) chirp
    );
}

// .. c:function::
static
void
_ch_cn_write_cb(uv_write_t* req, int status)
//    :noindex:
//
//    see: :c:func:`_ch_cn_write_cb`
//
// .. code-block:: cpp
//
{
    ch_connection_t* conn = req->data;
    ch_chirp_t* chirp = conn->chirp;
    A(chirp->_init == CH_CHIRP_MAGIC, "Not a ch_chirp_t*");
#   ifndef NDEBUG
        conn->flags &= ~CH_CN_WRITE_PENDING;
        conn->flags &= ~CH_CN_BUF_WTLS_USED;
#   endif
    if(status < 0) {
        L(
            chirp,
            "Sending pending data failed. ch_chirp_t:%p, ch_connection_t:%p",
            (void*) chirp,
            (void*) conn
        );
        conn->write_callback(req, status);
        ch_cn_shutdown(conn, status);
        return;
    }
    if(conn->write_size < conn->write_written) {
        _ch_cn_partial_write(conn);
        L(
            chirp,
            "Partially encrypted %d of %d bytes. "
            "ch_chirp_t:%p, ch_connection_t:%p",
            (int) conn->write_written,
            (int) conn->write_size,
            (void*) chirp,
            (void*) conn
        );
    } else {
        int pending = BIO_pending(conn->bio_app);
        if(pending < 1) {
            L(
                chirp,
                "Completely sent %d bytes. ch_chirp_t:%p, ch_connection_t:%p",
                (int) conn->write_written,
                (void*) chirp,
                (void*) conn
            );
            conn->write_size = 0;
            if(conn->write_callback != NULL)
                conn->write_callback(req, status);
        } else {
#           ifndef NDEBUG
                conn->flags |= CH_CN_WRITE_PENDING;
                conn->flags |= CH_CN_BUF_WTLS_USED;
#           endif
            L(
                chirp,
                "Partially sent %d bytes. "
                "ch_chirp_t:%p, ch_connection_t:%p",
                (int) conn->buffer_size,
                (void*) chirp,
                (void*) conn
            );
            int read = BIO_read(
                conn->bio_app,
                conn->buffer_wtls,
                conn->buffer_size
            );
            conn->buffer_wtls_uv.len = read;
            uv_write(
                &conn->write_req,
                (uv_stream_t*) &conn->client,
                &conn->buffer_wtls_uv,
                1,
                _ch_cn_write_cb
            );
            L(
                chirp,
                "Called uv_write with %d bytes. ch_chirp_t:%p, "
                "ch_connection_t:%p",
                (int) read,
                (void*) chirp,
                (void*) conn
            );
        }
    }
}

// .. c:function::
void
ch_cn_close_cb(uv_handle_t* handle)
//    :noindex:
//
//    see: :c:func:`ch_cn_close_cb`
//
// .. code-block:: cpp
//
{
    ch_connection_t* conn = handle->data;
    ch_chirp_t* chirp = conn->chirp;
    A(chirp->_init == CH_CHIRP_MAGIC, "Not a ch_chirp_t*");
    ch_chirp_int_t* ichirp = chirp->_;
    conn->shutdown_tasks -= 1;
    A(conn->shutdown_tasks > -1, "Shutdown semaphore dropped below zero");
    L(
        chirp,
        "Shutdown semaphore (%d). ch_connection_t:%p, ch_chirp_t:%p",
        conn->shutdown_tasks,
        (void*) conn,
        (void*) chirp
    );
    /* In production we allow the semaphore to drop below 0, but we log an
     * error
     */
    if(conn->shutdown_tasks < 0) {
        E(
            chirp,
            "Shutdown semaphore dropped blow 0. ch_chirp_t:%p",
            (void*) chirp
        );
    }
    if(conn->shutdown_tasks < 1) {
        if(ichirp->flags & CH_CHIRP_CLOSING)
            chirp->_->closing_tasks -= 1;
        if(conn->buffer_uv != NULL) {
            ch_free(conn->buffer_uv);
            if(conn->flags & CH_CN_ENCRYPTED) {
                ch_free(conn->buffer_wtls);
                ch_free(conn->buffer_rtls);
            }
        }
        if(conn->ssl != NULL)
            /* The doc says this frees conn->bio_ssl I tested it. let's
             * hope they never change that.
             */
            SSL_free(conn->ssl);
        if(conn->bio_app != NULL)
            BIO_free(conn->bio_app);
        ch_free(conn);
        L(
            chirp,
            "Closed connection, closing semaphore (%d). ch_connection_t:%p, "
            "ch_chirp_t:%p",
            chirp->_->closing_tasks,
            (void*) conn,
            (void*) chirp
        );
    }
}

// .. c:function::
ch_error_t
ch_cn_init(ch_chirp_t* chirp, ch_connection_t* conn, uint8_t flags)
//    :noindex:
//
//    see: :c:func:`ch_cn_init`
//
// .. code-block:: cpp
//
{
    int tmp_err;

    A(chirp->_init == CH_CHIRP_MAGIC, "Not a ch_chirp_t*");
    ch_chirp_int_t* ichirp  = chirp->_;
    conn->load            = -1;
    conn->chirp           = chirp;
    conn->flags          |= flags;
    conn->write_req.data  = conn;
    tmp_err = ch_rd_init(&conn->reader, ichirp->config.MAX_HANDLERS);
    if(tmp_err != CH_SUCCESS) {
        E(
            chirp,
            "Could not initialize reader: %d. ch_connection_t:%p,"
            " ch_chirp_t:%p",
            tmp_err,
            (void*) conn,
            (void*) chirp
        );
        return tmp_err;
    }
    ch_wr_init(&conn->writer, conn);
    tmp_err = uv_timer_init(ichirp->loop, &conn->shutdown_timeout);
    if(tmp_err != CH_SUCCESS) {
        E(
            chirp,
            "Initializing shutdown timeout failed: %d. ch_connection_t:%p,"
            " ch_chirp_t:%p",
            tmp_err,
            (void*) conn,
            (void*) chirp
        );
        return tmp_err;
    }
    conn->shutdown_timeout.data = conn;
    if(conn->flags & CH_CN_ENCRYPTED)
        tmp_err = ch_cn_init_enc(chirp, conn);
    if(tmp_err != CH_SUCCESS)
        return tmp_err;
    return _ch_cn_allocate_buffers(conn);
}

// .. c:function::
ch_error_t
ch_cn_init_enc(ch_chirp_t* chirp, ch_connection_t* conn)
//    :noindex:
//
//    see: :c:func:`ch_cn_init_enc`
//
// .. code-block:: cpp
//
{
    A(chirp->_init == CH_CHIRP_MAGIC, "Not a ch_chirp_t*");
    ch_chirp_int_t* ichirp = chirp->_;
    conn->ssl = SSL_new(ichirp->encryption.ssl_ctx);
    if(conn->ssl == NULL) {
#       ifndef NDEBUG
            ERR_print_errors_fp(stderr);
#       endif
        E(
            chirp,
            "Could not create SSL. ch_chirp_t:%p, ch_connection_t:%p",
            (void*) chirp,
            (void*) conn
        );
        return CH_TLS_ERROR;
    }
    if(BIO_new_bio_pair(&(conn->bio_ssl), 0, &(conn->bio_app), 0) != 1) {
#       ifndef NDEBUG
            ERR_print_errors_fp(stderr);
#       endif
        E(
            chirp,
            "Could not create BIO pair. ch_chirp_t:%p, ch_connection_t:%p",
            (void*) chirp,
            (void*) conn
        );
        SSL_free(conn->ssl);
        return CH_TLS_ERROR;
    }
    SSL_set_bio(conn->ssl, conn->bio_ssl, conn->bio_ssl);
#   ifdef CH_CN_PRINT_CIPHERS
    STACK_OF(SSL_CIPHER)* ciphers = SSL_get_ciphers(conn->ssl);
    while(sk_SSL_CIPHER_num(ciphers) > 0) {
        fprintf(
            stderr,
            "%s\n",
            SSL_CIPHER_get_name(sk_SSL_CIPHER_pop(ciphers))
        );
    }
    sk_SSL_CIPHER_free(ciphers);

#   endif
    L(
        chirp,
        "SSL context created. ch_chirp_t:%p, ch_connection_t:%p",
        (void*) chirp,
        (void*) conn
    );
    return CH_SUCCESS;
}

// .. c:function::
void
ch_cn_read_alloc_cb(
        uv_handle_t* handle,
        size_t suggested_size,
        uv_buf_t* buf
)
//    :noindex:
//
//    see: :c:func:`ch_cn_read_alloc_cb`
//
// .. code-block:: cpp
//
{
    /* That whole suggested size concept doesn't work, we have to allocated
     * consistent buffers.
     */
    (void)(suggested_size);
    ch_connection_t* conn = handle->data;
    ch_chirp_t* chirp = conn->chirp;
    A(chirp->_init == CH_CHIRP_MAGIC, "Not a ch_chirp_t*");
    // ichirp->config.BUFFER_SIZE = 40; // TODO remove
    A(!(conn->flags & CH_CN_BUF_UV_USED), "UV buffer still used");
#   ifndef NDEBUG
        conn->flags |= CH_CN_BUF_UV_USED;
#   endif
    buf->base = conn->buffer_uv;
    buf->len = conn->buffer_size;
}

// .. c:function::
void
ch_cn_send_if_pending(ch_connection_t* conn)
//    :noindex:
//
//    see: :c:func:`ch_cn_send_if_pending`
//
// .. code-block:: cpp
//
{
    A(!(conn->flags & CH_CN_WRITE_PENDING), "Another write is still pending");
    ch_chirp_t* chirp = conn->chirp;
    A(chirp->_init == CH_CHIRP_MAGIC, "Not a ch_chirp_t*");
    int pending = BIO_pending(conn->bio_app);
    if(pending < 1) {
        if(!(conn->flags & CH_CN_TLS_HANDSHAKE))
            ch_rd_read(conn, NULL, 0); // Start reader
        return;
    }
    A(!(conn->flags & CH_CN_BUF_WTLS_USED), "The wtls buffer is still used");
#   ifndef NDEBUG
        conn->flags |= CH_CN_BUF_WTLS_USED;
        conn->flags |= CH_CN_WRITE_PENDING;
#   endif
    int read = BIO_read(conn->bio_app, conn->buffer_wtls, conn->buffer_size);
    conn->buffer_wtls_uv.len = read;
    uv_write(
        &conn->write_req,
        (uv_stream_t*) &conn->client,
        &conn->buffer_wtls_uv,
        1,
        _ch_cn_send_pending_cb
    );
    L(
        chirp,
        "Sending %d pending handshake bytes. ch_chirp_t:%p, "
        "ch_connection_t:%p",
        read,
        (void*) chirp,
        (void*) conn
    );
}

// .. c:function::
ch_error_t
ch_cn_shutdown(
    ch_connection_t* conn,
    int reason
)
//    :noindex:
//
//    see: :c:func:`ch_cn_shutdown`
//
// .. code-block:: cpp
//
{
    int tmp_err;
    ch_chirp_t* chirp = conn->chirp;
    A(chirp->_init == CH_CHIRP_MAGIC, "Not a ch_chirp_t*");
    ch_chirp_int_t* ichirp = chirp->_;
    ch_protocol_t* protocol = &ichirp->protocol;
    ch_writer_t* writer = &conn->writer;
    ch_message_t* msg = writer->msg;
    if(msg != NULL) {
        writer->msg = NULL;
        if(msg->_send_cb != NULL) {
            // The user may free the message in the cb
            ch_send_cb_t cb = msg->_send_cb;
            msg->_send_cb = NULL;
            cb(
                msg,
                reason,
                conn->load
            );
        }
    }
    if(conn->flags & CH_CN_SHUTTING_DOWN) {
        E(
            chirp,
            "Shutdown in progress. ch_connection_t:%p, ch_chirp_t:%p",
            (void*) conn,
            (void*) chirp
        );
        return CH_IN_PRORESS;
    }
    /* There are many reasons the connection is not in this data-structure,
     * therefore we do a blind delete.
     */
    ch_connection_t* out_conn;
    sglib_ch_connection_t_delete_if_member(
        &protocol->connections,
        conn,
        &out_conn
    );
    conn->flags |= CH_CN_SHUTTING_DOWN;
    if(conn->flags & CH_CN_ENCRYPTED) {
        tmp_err = SSL_get_verify_result(conn->ssl);
        if(tmp_err != X509_V_OK) {
            E(
                chirp,
                "Connection has cert verification error: %d. "
                "ch_connection_t:%p, ch_chirp_t:%p",
                tmp_err,
                (void*) conn,
                (void*) chirp
            );
        }
        // If we have a valid SSL connection send a shutdown to the remote
        if(SSL_is_init_finished(conn->ssl)) {
            if(SSL_shutdown(conn->ssl) < 0) {
                E(
                    chirp,
                    "Could not shutdown SSL connection. "
                    "ch_connection_t:%p, ch_chirp_t:%p",
                    (void*) conn,
                    (void*) chirp
                );
            } else
                ch_cn_send_if_pending(conn);
        }
    }
    tmp_err = uv_shutdown(
        &conn->shutdown_req,
        (uv_stream_t*) &conn->client,
        _ch_cn_shutdown_cb
    );
    if(tmp_err != CH_SUCCESS) {
        E(
            chirp,
            "uv_shutdown returned error: %d. ch_connection_t:%p, "
            "ch_chirp_t:%p",
            tmp_err,
            (void*) conn,
            (void*) chirp
        );
        return ch_uv_error_map(tmp_err);
    }
    if(ichirp->flags & CH_CHIRP_CLOSING)
        chirp->_->closing_tasks += 1;
    tmp_err = uv_timer_start(
        &conn->shutdown_timeout,
        _ch_cn_shutdown_timeout_cb,
        ichirp->config.TIMEOUT * 1000,
        0
    );
    if(tmp_err != CH_SUCCESS) {
        E(
            chirp,
            "Starting shutdown timeout failed: %d. ch_connection_t:%p,"
            " ch_chirp_t:%p",
            tmp_err,
            (void*) conn,
            (void*) chirp
        );
    }
    L(
        chirp,
        "Shutdown connection. ch_connection_t:%p, ch_chirp_t:%p",
        (void*) conn,
        (void*) chirp
    );
    return CH_SUCCESS;
}

// .. c:function::
void
ch_cn_write(
        ch_connection_t* conn,
        void* buf,
        size_t size,
        uv_write_cb callback
)
//    :noindex:
//
//    see: :c:func:`ch_cn_write`
//
// .. code-block:: cpp
//
{
    ch_chirp_t* chirp = conn->chirp;
    A(chirp->_init == CH_CHIRP_MAGIC, "Not a ch_chirp_t*");
    A(conn->write_size == 0, "Another connection write is pending");
    if(conn->flags & CH_CN_ENCRYPTED) {
        conn->write_callback  = callback;
        conn->write_buffer    = buf;
        conn->write_size      = size;
        conn->write_written   = 0;
#       ifndef NDEBUG
            int pending = BIO_pending(conn->bio_app);
            A(pending == 0, "There is still pending data in SSL BIO");
#       endif
        _ch_cn_partial_write(conn);
    } else {
        conn->buffer_any_uv = uv_buf_init(buf, size);
        uv_write(
            &conn->write_req,
            (uv_stream_t*) &conn->client,
            &conn->buffer_any_uv,
            1,
            _ch_cn_write_cb
        );
        L(
            chirp,
            "Called uv_write with %d bytes. ch_chirp_t:%p, ch_connection_t:%p",
            (int) size,
            (void*) chirp,
            (void*) conn
        );
    }
}
