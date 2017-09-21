// ==========
// Connection
// ==========
//

// Project includes
// ================
//
// .. code-block:: cpp
//
#include "connection.h"
#include "chirp.h"
#include "util.h"
#include "remote.h"

// System includes
// ===============
//
// .. code-block:: cpp
//
#include <openssl/err.h>

// Data Struc Prototypes
// =====================

// .. code-block:: cpp
//
qs_stack_bind_impl_m(ch_cn_old, ch_connection_t)

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
//    Called during handshake by libuv when pending data is sent.
//
//    :param uv_write_t* req: Write request type, holding the
//                            connection handle
//    :param int status: Send status
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
// .. code-block:: cpp

MINMAX_FUNCS(size_t)

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
    conn->buffer_size = size;
    if(conn->flags & CH_CN_ENCRYPTED) {
        conn->buffer_wtls  = ch_alloc(size);
        size = min_size_t(size, CH_ENC_BUFFER_SIZE);
        conn->buffer_rtls  = ch_alloc(size);
    }
    conn->buffer_rtls_size = size;
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
        EC(
            chirp,
            "Could not allocate memory for libuv and tls. ",
            "ch_connection_t:%p",
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
    for(;;) {
        int can_write_more = 1;
        int pending = BIO_pending(conn->bio_app);
        while(pending && can_write_more) {
            int read = BIO_read(
                conn->bio_app,
                conn->buffer_wtls + bytes_read,
                conn->buffer_size - bytes_read
            );
            A(read > 0, "BIO_read failure unexpected");
            if(read < 1) {
                EC(
                    chirp,
                    "SSL error reading from BIO, shutting down connection. ",
                    "ch_connection_t:%p",
                    (void*) conn
                );
                ch_cn_shutdown(conn, CH_TLS_ERROR);
                return;
            }
            bytes_read += read;
            int is_write_size_valid = (
                bytes_encrypted + conn->write_written
            ) < conn->write_size;
            int is_buffer_size_valid = bytes_read < conn->buffer_size;

            can_write_more = is_write_size_valid && is_buffer_size_valid;
            pending = BIO_pending(conn->bio_app);
        }
        if(!can_write_more)
            break;
        int tmp_err = SSL_write(
            conn->ssl,
            conn->write_buffer + bytes_encrypted + conn->write_written,
            conn->write_size - bytes_encrypted - conn->write_written
        );
        bytes_encrypted += tmp_err;
        A(tmp_err > -1, "SSL_write failure unexpected");
        if(tmp_err < 0) {
            EC(
                chirp,
                "SSL error writing to BIO, shutting down connection. ",
                "ch_connection_t:%p",
                (void*) conn
            );
            ch_cn_shutdown(conn, CH_TLS_ERROR);
            return;
        }
    }
    conn->buffer_wtls_uv.len = bytes_read;
    uv_write(
        &conn->write_req,
        (uv_stream_t*) &conn->client,
        &conn->buffer_wtls_uv,
        1,
        _ch_cn_write_cb
    );
    LC(
        chirp,
        "Called uv_write with %d bytes. ",
        "ch_connection_t:%p",
        (int) bytes_read,
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
        LC(
            chirp,
            "Sending pending data failed. ", "ch_connection_t:%p",
            (void*) conn
        );
        ch_cn_shutdown(conn, status);
        return;
    }
    if(conn->flags & CH_CN_SHUTTING_DOWN) {
        LC(
            chirp,
            "Write shutdown bytes to connection successful. ",
            "ch_connection_t:%p",
            (void*) conn
        );
    } else {
        LC(
            chirp,
            "Write handshake bytes to connection successful. ",
            "ch_connection_t:%p",
            (void*) conn
        );
    }
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
    A(chirp->_init == CH_CHIRP_MAGIC, "Not a ch_chirp_t*");
    LC(
        chirp,
        "Shutdown callback called. ", "ch_connection_t:%p",
        (void*) conn
    );
    tmp_err = uv_timer_stop(&conn->shutdown_timeout);
    if(tmp_err != CH_SUCCESS) {
        EC(
            chirp,
            "Stopping shutdown timeout failed: %d. ", "ch_connection_t:%p",
            tmp_err,
            (void*) chirp
        );
    }
    uv_handle_t* handle = (uv_handle_t*) req->handle;
    if(uv_is_closing(handle)) {
        EC(
            chirp,
            "Connection already closed after shutdown. ",
            "ch_connection_t:%p",
            (void*) conn
        );
    } else {
        uv_read_stop(req->handle);
        ch_wr_free(&conn->writer);
        uv_close(handle, ch_cn_close_cb);
        uv_close((uv_handle_t*) &conn->shutdown_timeout, ch_cn_close_cb);
        conn->shutdown_tasks += 2;
        LC(
            chirp,
            "Closing connection after shutdown. ",
            "ch_connection_t:%p",
            (void*) conn
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
        EC(
            chirp,
            "Canceling shutdown timeout failed: %d. ", "ch_connection_t:%p",
            tmp_err,
            (void*) conn
        );
    }
    LC(
        chirp,
        "Shutdown timed out closing. ", "ch_connection_t:%p",
        (void*) conn
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
        LC(
            chirp,
            "Sending pending data failed. ", "ch_connection_t:%p",
            (void*) conn
        );
        uv_write_cb cb = conn->write_callback;
        conn->write_callback = NULL;
        cb(req, status);
        ch_cn_shutdown(conn, status);
        return;
    }
    /* Check if we can write data */
    int pending = BIO_pending(conn->bio_app);
    if(conn->write_size > conn->write_written || pending) {
        _ch_cn_partial_write(conn);
        LC(
            chirp,
            "Partially encrypted %d of %d bytes. ",
            "ch_connection_t:%p",
            (int) conn->write_written,
            (int) conn->write_size,
            (void*) conn
        );
    } else {
#       ifndef NDEBUG
            A(pending == 0, "Unexpected pending data on TLS write");
#       endif
        LC(
            chirp,
            "Completely sent %d bytes (unenc). ", "ch_connection_t:%p",
            (int) conn->write_written,
            (void*) conn
        );
        conn->write_written = 0;
        conn->write_size = 0;
        if(conn->write_callback != NULL) {
            uv_write_cb cb = conn->write_callback;
            conn->write_callback = NULL;
            cb(req, status);
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
    LC(
        chirp,
        "Shutdown semaphore (%d). ", "ch_connection_t:%p",
        conn->shutdown_tasks,
        (void*) conn
    );
    /* In production we allow the semaphore to drop below 0, but we log an
     * error. */
    if(conn->shutdown_tasks < 0) {
        E(
            chirp,
            "Shutdown semaphore dropped blow 0%s", ""
        );
    }
    if(conn->shutdown_tasks < 1) {
        if(conn->flags & CH_CN_DO_CLOSE_ACCOUTING)
            ichirp->closing_tasks -= 1;
        if(conn->buffer_uv != NULL) {
            ch_free(conn->buffer_uv);
            if(conn->flags & CH_CN_ENCRYPTED) {
                ch_free(conn->buffer_wtls);
                ch_free(conn->buffer_rtls);
            }
        }
        if(conn->ssl != NULL)
            /* The doc says this frees conn->bio_ssl I tested it. let's
             * hope they never change that. */
            SSL_free(conn->ssl);
        if(conn->bio_app != NULL)
            BIO_free(conn->bio_app);
        ch_free(conn);
        LC(
            chirp,
            "Closed connection, closing semaphore (%d). ", "ch_connection_t:%p",
            chirp->_->closing_tasks,
            (void*) conn
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
    ch_rd_init(&conn->reader);
    ch_wr_init(&conn->writer, conn);
    tmp_err = uv_timer_init(ichirp->loop, &conn->shutdown_timeout);
    if(tmp_err != CH_SUCCESS) {
        EC(
            chirp,
            "Initializing shutdown timeout failed: %d. ", "ch_connection_t:%p",
            tmp_err,
            (void*) conn
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
        EC(
            chirp,
            "Could not create SSL. ", "ch_connection_t:%p",
            (void*) conn
        );
        return CH_TLS_ERROR;
    }
    if(BIO_new_bio_pair(&(conn->bio_ssl), 0, &(conn->bio_app), 0) != 1) {
#       ifndef NDEBUG
            ERR_print_errors_fp(stderr);
#       endif
        EC(
            chirp,
            "Could not create BIO pair. ", "ch_connection_t:%p",
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
    LC(
        chirp,
        "SSL context created. ", "ch_connection_t:%p",
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
     * consistent buffers. */
    (void)(suggested_size);
    ch_connection_t* conn = handle->data;
    ch_chirp_t* chirp = conn->chirp;
    A(chirp->_init == CH_CHIRP_MAGIC, "Not a ch_chirp_t*");
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
        if(!(
                conn->flags & CH_CN_TLS_HANDSHAKE ||
                conn->flags & CH_CN_SHUTTING_DOWN
        ))
            ch_rd_read(conn, NULL, 0); /* Start the reader */
        return;
    }
    A(!(conn->flags & CH_CN_BUF_WTLS_USED), "The wtls buffer is still used");
#   ifndef NDEBUG
        conn->flags |= CH_CN_BUF_WTLS_USED;
        conn->flags |= CH_CN_WRITE_PENDING;
#   endif
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
        _ch_cn_send_pending_cb
    );
    if(conn->flags & CH_CN_SHUTTING_DOWN) {
        LC(
            chirp,
            "Sending %d pending shutdown bytes. ", "ch_connection_t:%p",
            read,
            (void*) conn
        );
    } else {
        LC(
            chirp,
            "Sending %d pending handshake bytes. ", "ch_connection_t:%p",
            read,
            (void*) conn
        );
    }
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
    ch_chirp_t* chirp = conn->chirp;
    A(chirp->_init == CH_CHIRP_MAGIC, "Not a ch_chirp_t*");
    if(conn->flags & CH_CN_SHUTTING_DOWN) {
        EC(
            chirp,
            "Shutdown in progress. ", "ch_connection_t:%p",
            (void*) conn
        );
        return CH_IN_PRORESS;
    }
    conn->flags |= CH_CN_SHUTTING_DOWN;
    int tmp_err;
    ch_chirp_int_t* ichirp = chirp->_;
    ch_protocol_t* protocol = &ichirp->protocol;
    ch_writer_t* writer = &conn->writer;
    ch_message_t* msg = writer->msg;
    /* # TODO this should use ch_remote_t directly from function signature */
    ch_remote_t search_remote;
    ch_remote_t* remote;
    ch_rm_init_from_conn(chirp, &search_remote, conn);
    ch_rm_find(
        protocol->remotes,
        &search_remote,
        &remote
    );
    remote->conn = NULL;
    LC(
        chirp,
        "Shutdown connection. ", "ch_connection_t:%p",
        (void*) conn
    );
    uv_read_stop((uv_stream_t*) &conn->client);
    if(msg != NULL) {
        msg->_flags = CH_MSG_FAILURE;
        ch_chirp_try_message_finish(
            chirp,
            conn,
            msg,
            reason,
            conn->load
        );
    }
    if(conn->flags & CH_CN_ENCRYPTED) {
        tmp_err = SSL_get_verify_result(conn->ssl);
        if(tmp_err != X509_V_OK) {
            EC(
                chirp,
                "Connection has cert verification error: %d. ",
                "ch_connection_t:%p",
                tmp_err,
                (void*) conn
            );
        }
        /* If we have a valid SSL connection send a shutdown to the remote */
        if(SSL_is_init_finished(conn->ssl)) {
            if(SSL_shutdown(conn->ssl) < 0) {
                EC(
                    chirp,
                    "Could not shutdown SSL connection. ",
                    "ch_connection_t:%p",
                    (void*) conn
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
        EC(
            chirp,
            "uv_shutdown returned error: %d. ", "ch_connection_t:%p",
            tmp_err,
            (void*) conn
        );
        return ch_uv_error_map(tmp_err);
    }
    if(ichirp->flags & CH_CHIRP_CLOSING) {
        conn->flags |= CH_CN_DO_CLOSE_ACCOUTING;
        chirp->_->closing_tasks += 1;
    }
    tmp_err = uv_timer_start(
        &conn->shutdown_timeout,
        _ch_cn_shutdown_timeout_cb,
        ichirp->config.TIMEOUT * 1000,
        0
    );
    if(tmp_err != CH_SUCCESS) {
        EC(
            chirp,
            "Starting shutdown timeout failed: %d. ", "ch_connection_t:%p",
            tmp_err,
            (void*) conn
        );
    }
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
            callback
        );
        LC(
            chirp,
            "Called uv_write with %d bytes. ", "ch_connection_t:%p",
            (int) size,
            (void*) conn
        );
    }
}
