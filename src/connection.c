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
ch_inline
ch_error_t
_ch_cn_shutdown_gen(
        ch_connection_t* conn,
        uv_shutdown_cb shutdown_cb,
        uv_timer_cb timer_cb
);
//
//    Generic version of shutdown, called by ch_cn_shutdown.
//
//    :param ch_connection_t* conn: Connection dictionary holding a
//                                  chirp instance.
//    :param uv_shutdown_cb shutdown_cb: Callback which gets called
//                                       after the shutdown is
//                                       complete
//    :param uv_timer_cb timer_cb: Callback which gets called after
//                                 the timer has reached 0
//    :return: A chirp error. see: :c:type:`ch_error_t`
//    :rtype: ch_error_t

// .. c:function::
static
ch_inline
void
_ch_cn_shutdown_gen_cb(
        uv_shutdown_t* req,
        int status,
        uv_close_cb close_cb
);
//
//    Generic version of the shutdown callback, called by ch_cn_shutdown_cb.
//
//    :param uv_shutdown_t* req: Shutdown request type, holding the
//                               connection handle
//    :param int status: The status after the shutdown. 0 in case of
//                       success, < 0 otherwise
//    :param uv_close_cb close_cb: Callback which will be called
//                                 asynchronously after uv_close
//                                 has been called

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
ch_inline
void
_ch_cn_shutdown_timeout_gen_cb(
        uv_timer_t* handle,
        uv_shutdown_cb shutdown_cb
);
//
//    Generic version of the shutdown callback, called by
//    _ch_cn_shutdown_timeout_cb.
//
//    :param uv_timer_t* handle: Timer handle to schedule callback
//    :param uv_shutdown_cb shutdown_cb: Callback which gets called
//                                       after the shutdown is
//                                       complete

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
            ch_cn_shutdown(conn);
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
        ch_cn_shutdown(conn);
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
_ch_cn_shutdown_cb(uv_shutdown_t* req, int status)
//    :noindex:
//
//    see: :c:func:`_ch_cn_shutdown_cb`
//
// .. code-block:: cpp
//
{
    _ch_cn_shutdown_gen_cb(
        req,
        status,
        ch_cn_close_cb
    );
}

// .. c:function::
static
ch_inline
void
_ch_cn_shutdown_gen_cb(
        uv_shutdown_t* req,
        int status,
        uv_close_cb close_cb
)
//    :noindex:
//
//    see: :c:func:`_ch_cn_shutdown_gen_cb`
//
// .. code-block:: cpp
//
{
    (void)(status); // Ignore callback-arg
    int tmp_err;
    ch_connection_t* conn = req->handle->data;
    ch_chirp_t* chirp = conn->chirp;
    ch_chirp_int_t* ichirp = chirp->_;
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
        if(ichirp->flags & CH_CHIRP_CLOSING)
            chirp->_->closing_tasks -= 1;
        E(
            chirp,
            "Connection already closed after shutdown. "
            "ch_connection_t:%p, ch_chirp_t:%p",
            (void*) conn,
            (void*) chirp
        );
    } else {
        uv_close((uv_handle_t*) req->handle, close_cb);
        uv_close((uv_handle_t*) &conn->shutdown_timeout, close_cb);
        if(ichirp->flags & CH_CHIRP_CLOSING)
            chirp->_->closing_tasks += 1;
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
ch_inline
ch_error_t
_ch_cn_shutdown_gen(
    ch_connection_t* conn,
    uv_shutdown_cb shutdown_cb,
    uv_timer_cb timer_cb
)
//    :noindex:
//
//    see: :c:func:`_ch_cn_shutdown_gen`
//
// .. code-block:: cpp
//
{
    int tmp_err;
    ch_chirp_t* chirp = conn->chirp;
    A(chirp->_init == CH_CHIRP_MAGIC, "Not a ch_chirp_t*");
    ch_chirp_int_t* ichirp = chirp->_;
    ch_protocol_t* protocol = &ichirp->protocol;
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
        shutdown_cb
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
        timer_cb,
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
static
void
_ch_cn_shutdown_timeout_cb(uv_timer_t* handle)
//    :noindex:
//
//    see: :c:func:`_ch_cn_shutdown_timeout_cb`
//
// .. code-block:: cpp
//
{
    _ch_cn_shutdown_timeout_gen_cb(
        handle,
        _ch_cn_shutdown_cb
    );
}

// .. c:function::
static
ch_inline
void
_ch_cn_shutdown_timeout_gen_cb(
        uv_timer_t* handle,
        uv_shutdown_cb shutdown_cb
)
//    :noindex:
//
//    see: :c:func:`_ch_cn_shutdown_timeout_gen_cb`
//
// .. code-block:: cpp
//
{
    ch_connection_t* conn = handle->data;
    int tmp_err;
    ch_chirp_t* chirp = conn->chirp;
    A(chirp->_init == CH_CHIRP_MAGIC, "Not a ch_chirp_t*");
    ch_chirp_int_t* ichirp = chirp->_;
    if(ichirp->flags & CH_CHIRP_CLOSING)
        chirp->_->closing_tasks -= 1;
    shutdown_cb(&conn->shutdown_req, 1);
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
        ch_cn_shutdown(conn);
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
    if(ichirp->flags & CH_CHIRP_CLOSING)
        chirp->_->closing_tasks -= 1;
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
        ch_rd_free(&conn->reader);
        ch_wr_free(&conn->writer);
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
    memset(conn, 0, sizeof(ch_connection_t));
    conn->load            = 0;
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
        return ch_cn_init_enc(chirp, conn);
    return CH_SUCCESS;
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
    ch_connection_t* conn = handle->data;
    ch_chirp_t* chirp = conn->chirp;
    A(chirp->_init == CH_CHIRP_MAGIC, "Not a ch_chirp_t*");
    ch_chirp_int_t* ichirp = chirp->_;
    // ichirp->config.BUFFER_SIZE = 40; // TODO remove
    A(!(conn->flags & CH_CN_BUF_UV_USED), "UV buffer still used");
#   ifndef NDEBUG
        conn->flags |= CH_CN_BUF_UV_USED;
#   endif
    if(!conn->buffer_uv) {
        /* We also allocate the TLS buffer, because they have to be of the same
         * size
         */
        if(ichirp->config.BUFFER_SIZE == 0) {
            conn->buffer_uv   = ch_alloc(suggested_size);
            if(conn->flags & CH_CN_ENCRYPTED) {
                conn->buffer_wtls  = ch_alloc(suggested_size);
                conn->buffer_rtls  = ch_alloc(suggested_size);
            }
            conn->buffer_size = suggested_size;
        } else {
            conn->buffer_uv   = ch_alloc(ichirp->config.BUFFER_SIZE);
            if(conn->flags & CH_CN_ENCRYPTED) {
                conn->buffer_wtls  = ch_alloc(ichirp->config.BUFFER_SIZE);
                conn->buffer_rtls  = ch_alloc(ichirp->config.BUFFER_SIZE);
            }
            conn->buffer_size = ichirp->config.BUFFER_SIZE;
        }
        if(!(conn->buffer_uv && conn->buffer_wtls && conn->buffer_rtls)) {
            E(
                chirp,
                "Could not allocate memory for libuv and tls. "
                "ch_chirp_t:%p, ch_connection_t:%p",
                (void*) chirp,
                (void*) conn
            );
            // Tell libuv about the error
            buf->base = 0;
            buf->len = 0;
            return;
        }
        conn->buffer_uv_uv = uv_buf_init(
            conn->buffer_uv,
            conn->buffer_size
        );
        conn->buffer_wtls_uv = uv_buf_init(
            conn->buffer_wtls,
            conn->buffer_size
        );
    }
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
ch_cn_shutdown(ch_connection_t* conn)
//    :noindex:
//
//    see: :c:func:`ch_cn_shutdown`
//
// .. code-block:: cpp
//
{
    return _ch_cn_shutdown_gen(
        conn,
        _ch_cn_shutdown_cb,
        _ch_cn_shutdown_timeout_cb
    );
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
