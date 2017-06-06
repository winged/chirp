// =====
// Chirp
// =====
//
// .. todo:: Document purpose
//

// Project includes
// ================
//
// .. code-block:: cpp
//
#include "chirp.h"
#include "util.h"
#include "structures.h"

// System includes
// ===============
//
// .. code-block:: cpp
//
#include <openssl/err.h>
#include <time.h>
#include <signal.h>
#ifdef _WIN32
#   define ch_access _access
#   include <io.h>
#else
#   define ch_access access
#   include <unistd.h>
#endif

// Sglib Prototypes
// ================

// .. code-block:: cpp
//
SGLIB_DEFINE_RBTREE_FUNCTIONS( // NOCOV
    ch_message_dest_t,
    _left,
    _right,
    _color_field,
    CH_MESSAGE_DEST_CMP
)

// Declarations
// ============
//
// .. c:var:: uv_mutex_t _ch_chirp_init_lock
//
//    It seems that initializing signals accesses a shared data-structure. We
//    lock during init, just to be sure.
//
// .. code-block:: cpp
//
static uv_mutex_t _ch_chirp_init_lock;

// .. c:var:: ch_config_t ch_config_defaults
//
//    Default config of chirp.
//
// .. code-block:: cpp
//
static ch_config_t _ch_config_defaults = {
    .REUSE_TIME         = 30,
    .TIMEOUT            = 5,
    .PORT               = 2998,
    .BACKLOG            = 100,
    .RETRIES            = 1,
    .MAX_HANDLERS       = 16,
    .FLOW_CONTROL       = 1,
    .ACKNOWLEDGE        = 1,
    .DISABLE_SIGNALS    = 0,
    .BUFFER_SIZE        = 0,
    .BIND_V6            = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    .BIND_V4            = {0, 0, 0, 0},
    .IDENTITY           = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    .CERT_CHAIN_PEM     = NULL,
    .DH_PARAMS_PEM      = NULL,
    .DISABLE_ENCRYPTION = 0,
};

// .. c:function::
static
void
_ch_chirp_check_closing_cb(uv_prepare_t* handle);
//
//    Close chirp when the closing semaphore reaches zero.
//
//    :param uv_prepare_t* handle: Prepare handle which will be stopped (and
//                                 thus its callback)
//

// .. c:function::
static
void
_ch_chirp_close_async_cb(uv_async_t* handle);
//
//    Internal callback to close chirp. Makes ch_chirp_close_ts thread-safe
//
//    :param uv_async_t* handle: Async handle which is used to closed chirp
//

// .. c:function::
static
void
_ch_chirp_closing_down_cb(uv_handle_t* handle);
//
//    Close chirp after the check callback has been closed and stops the libuv
//    loop.
//
//    :param uv_handle_t* handle: Base libuv handle which contains chirp (as
//                                data)
//

// .. c:function::
static
void
_ch_chirp_done(uv_async_t* handle);
//
//    Done callback calls the user supplied done callback.
//

// .. c:function::
static
void
_ch_chirp_init_signals(ch_chirp_t *chirp);
//
//    Setup signal handlers for chirp. Internally called from ch_chirp_init()
//
//   :param   ch_chirp_t* chrip: Instance of a chirp object


// .. c:function::
static
void
_ch_chirp_sig_handler(uv_signal_t* ,int);
//
//    Closes all chirp instances on sig int.
//
//    :param uv_signal_t* handle : The libuv signal handler structure
//
//    :param int signo: The signal number, that tells which signal should be
//                      handled.
//

// .. c:function::
static
void
_ch_chirp_start(uv_async_t* handle);
//
//    Start callback calls the user supplied done callback.
//
//    :param uv_handle_t* handle: Async handler.
//

// .. c:function::
static
ch_error_t
_ch_chirp_verify_cfg(const ch_chirp_t* chirp);
//
//   Verifies the configuration.
//
//   :param   ch_chirp_t* chrip: Instance of a chirp object
//
//   :return: A chirp error. see: :c:type:`ch_error_t`
//   :rtype:  ch_error_t


// .. c:var:: extern char* ch_version
//    :noindex:
//
//    Version of chirp.
//
// .. code-block:: cpp
//
CH_EXPORT
char* ch_version = CH_VERSION;

// Definitions
// ===========

// .. c:function::
static
void
_ch_chirp_check_closing_cb(uv_prepare_t* handle)
//    :noindex:
//
//    see: :c:func:`ch_chirp_close_cb`
//
// .. code-block:: cpp
//
{
    ch_chirp_t* chirp = handle->data;
    A(chirp->_init == CH_CHIRP_MAGIC, "Not a ch_chirp_t*");
    ch_chirp_int_t* ichirp = chirp->_;
    A(ichirp->closing_tasks > -1, "Closing semaphore dropped below zero");
    L(
        chirp,
        "Check closing semaphore (%d). ch_chirp_t:%p",
        ichirp->closing_tasks,
        (void*) chirp
    );
    /* In production we allow the semaphore to drop below zero but log it as
     * an error. */
    if(ichirp->closing_tasks < 1) {
        int tmp_err;
        tmp_err = uv_prepare_stop(handle);
        A(tmp_err == CH_SUCCESS, "Could not stop prepare callback");
        if(!ichirp->config.DISABLE_ENCRYPTION) {
            tmp_err = ch_en_stop(&ichirp->encryption);
            A(tmp_err == CH_SUCCESS, "Could not stop encryption");
        }
        uv_close((uv_handle_t*) handle, _ch_chirp_closing_down_cb);
    }
    if(ichirp->closing_tasks < 0) {
        E(
            chirp,
            "Check closing semaphore dropped blow 0. ch_chirp_t:%p",
            (void*) chirp
        );
    }
}

// .. c:function::
static
void
_ch_chirp_close_async_cb(uv_async_t* handle)
//    :noindex:
//
//    see: :c:func:`_ch_chirp_close_async_cb`
//
// .. code-block:: cpp
//
{
    int tmp_err;
    ch_chirp_t* chirp = handle->data;
    A(chirp->_init == CH_CHIRP_MAGIC, "Not a ch_chirp_t*");
    if(chirp->_ == NULL) {
        E(
            chirp,
            "Chirp closing callback called on closed. ch_chirp_t:%p",
            (void*) chirp
        );
        return;
    }
    ch_chirp_int_t* ichirp = chirp->_;
    if(ichirp->flags & CH_CHIRP_CLOSED) {
        E(
            chirp,
            "Chirp closing callback called on closed. ch_chirp_t:%p",
            (void*) chirp
        );
        return;
    }
    L(
        chirp,
        "Chirp closing callback called. ch_chirp_t:%p",
        (void*) chirp
    );
    tmp_err = ch_pr_stop(&ichirp->protocol);
    A(tmp_err == CH_SUCCESS, "Could not stop protocol");
    if(!ichirp->config.DISABLE_SIGNALS) {
        uv_signal_stop(&ichirp->signals[0]);
        uv_signal_stop(&ichirp->signals[1]);
        uv_close((uv_handle_t*) &ichirp->signals[0], ch_chirp_close_cb);
        uv_close((uv_handle_t*) &ichirp->signals[1], ch_chirp_close_cb);
        ichirp->closing_tasks += 2;
    }
    uv_close((uv_handle_t*) &ichirp->send_ts, ch_chirp_close_cb);
    uv_close((uv_handle_t*) &ichirp->close, ch_chirp_close_cb);
    ichirp->closing_tasks += 2;
    tmp_err = uv_prepare_init(ichirp->loop, &ichirp->close_check);
    A(tmp_err == CH_SUCCESS, "Could not init prepare callback");
    ichirp->close_check.data = chirp;
    /* We use a semaphore to wait until all callbacks are done:
     * 1. Every time a new callback is scheduled we do
     *    ichirp->closing_tasks += 1
     * 2. Every time a callback is called we do ichirp->closing_tasks -= 1
     * 3. Every uv_loop iteration before it blocks we check
     *    ichirp->closing_tasks == 0
     * -> if we reach 0 all callbacks are done and we continue freeing memory
     * etc. */
    tmp_err = uv_prepare_start(
        &ichirp->close_check,
        _ch_chirp_check_closing_cb
    );
    A(tmp_err == CH_SUCCESS, "Could not start prepare callback");
}

// .. c:function::
void
ch_chirp_close_cb(uv_handle_t* handle)
//    :noindex:
//
//    see: :c:func:`ch_chirp_close_cb`
//
// .. code-block:: cpp
//
{
    ch_chirp_t* chirp = handle->data;
    A(chirp->_init == CH_CHIRP_MAGIC, "Not a ch_chirp_t*");
    chirp->_->closing_tasks -= 1;
    L(
        chirp,
        "Closing semaphore (%d). uv_handle_t:%p, ch_chirp_t:%p",
        chirp->_->closing_tasks,
        (void*) handle,
        (void*) chirp
    );
}

// .. c:function::
static
void
_ch_chirp_closing_down_cb(uv_handle_t* handle)
//    :noindex:
//
//    see: :c:func:`_ch_chirp_closing_down_cb`
//
// .. code-block:: cpp
//
{
    ch_chirp_t* chirp = handle->data;
    A(chirp->_init == CH_CHIRP_MAGIC, "Not a ch_chirp_t*");
    ch_chirp_int_t* ichirp = chirp->_;
    uv_async_send(&chirp->_done);
    if(ichirp->flags & CH_CHIRP_AUTO_STOP) {
        uv_stop(ichirp->loop);
        L(
            chirp,
            "UV-Loop stopped by chirp. uv_loop_t:%p ch_chirp_t:%p",
            (void*) ichirp->loop,
            (void*) chirp
        );
    }
    uv_mutex_destroy(&ichirp->send_ts_queue_lock);
    chirp->_ = NULL;
    ch_free(ichirp);
    L(chirp, "Closed. ch_chirp_t:%p", (void*) chirp);
}

// .. c:function::
static
void
_ch_chirp_done(uv_async_t* handle)
//    :noindex:
//
//    see: :c:func:`_ch_chirp_done`
//
// .. code-block:: cpp
//
{
    ch_chirp_t* chirp = handle->data;
    A(chirp->_init == CH_CHIRP_MAGIC, "Not a ch_chirp_t*");
    uv_close((uv_handle_t*) handle, NULL);
    if(chirp->_done_cb != NULL)
        chirp->_done_cb(chirp);
}

// .. c:function::
static
void
_ch_chirp_init_signals(ch_chirp_t* chirp)
//    :noindex:
//
//    see: :c:func:`_ch_chirp_init_signals`
//
// .. code-block:: cpp
//
{
#   ifndef CH_DISABLE_SIGNALS
        ch_chirp_int_t* ichirp = chirp->_;
        if(ichirp->config.DISABLE_SIGNALS)
            return;
        uv_signal_init(ichirp->loop, &ichirp->signals[0]);
        uv_signal_init(ichirp->loop, &ichirp->signals[1]);

        ichirp->signals[0].data = chirp;
        ichirp->signals[1].data = chirp;

        if(uv_signal_start(
                &ichirp->signals[0],
                &_ch_chirp_sig_handler,
                SIGINT
        )) {
            E(
                chirp,
                "Unable to set SIGINT handler. ch_chirp_t:%p",
                (void*) chirp
            );
            return;
        }

        if(uv_signal_start(
                &ichirp->signals[1],
                &_ch_chirp_sig_handler,
                SIGTERM
        )) {
            uv_signal_stop(&ichirp->signals[0]);
            uv_close((uv_handle_t*) &ichirp->signals[0], NULL);
            E(
                chirp,
                "Unable to set SIGTERM handler. ch_chirp_t:%p",
                (void*) chirp
            );
        }
#   else
        (void)(chirp);
#   endif
}


// .. c:function::
static
void
_ch_chirp_sig_handler(uv_signal_t* handle, int signo)
//    :noindex:
//
//    see: :c:func:`_ch_chirp_sig_handler`
//
// .. code-block:: cpp
//
{
    ch_chirp_t* chirp = handle->data;
    A(chirp->_init == CH_CHIRP_MAGIC, "Not a ch_chirp_t*");

    if(signo != SIGINT && signo != SIGTERM)
        return;

    ch_chirp_close_ts(chirp);
}


// .. c:function::
static
void
_ch_chirp_start(uv_async_t* handle)
//    :noindex:
//
//    see: :c:func:`_ch_chirp_start`
//
// .. code-block:: cpp
//
{
    ch_chirp_t* chirp = handle->data;
    A(chirp->_init == CH_CHIRP_MAGIC, "Not a ch_chirp_t*");
    ch_chirp_int_t* ichirp = chirp->_;
    uv_close((uv_handle_t*) handle, NULL);
    if(ichirp->start_cb != NULL)
        ichirp->start_cb(chirp);
}

// .. c:function::
static
ch_error_t
_ch_chirp_verify_cfg(const ch_chirp_t* chirp)
//    :noindex:
//
//    see: :c:func:`_ch_chirp_verify_cfg`
//
// .. code-block:: cpp
//
{
    A(chirp->_init == CH_CHIRP_MAGIC, "Not a ch_chirp_t*");
    ch_config_t* conf = &chirp->_->config;
    V(
        chirp,
        conf->DH_PARAMS_PEM != NULL,
        "Config: DH_PARAMS_PEM must be set."
    );
    V(
        chirp,
        conf->CERT_CHAIN_PEM != NULL,
        "Config: CERT_CHAIN_PEM must be set."
    );
    V(
        chirp,
        ch_access(conf->CERT_CHAIN_PEM, F_OK ) != -1,
        "Config: cert %s does not exist.",
        conf->CERT_CHAIN_PEM
    );
    V(
        chirp,
        conf->PORT > 1024,
        "Config: port must be > 1042. (%d)",
        conf->PORT
    );
    V(
        chirp,
        conf->BACKLOG < 128,
        "Config: backlog must be < 128. (%d)",
        conf->BACKLOG
    );
    V(
        chirp,
        conf->TIMEOUT <= 60,
        "Config: timeout must be <= 60. (%f)",
        conf->TIMEOUT
    );
    V(
        chirp,
        conf->TIMEOUT >= 0.1,
        "Config: timeout must be >= 0.1. (%f)",
        conf->TIMEOUT
    );
    V(
        chirp,
        conf->REUSE_TIME >= 2,
        "Config: resuse time must be => 2. (%f)",
        conf->REUSE_TIME
    );
    V(
        chirp,
        conf->REUSE_TIME <= 3600,
        "Config: resuse time must be <= 3600. (%f)",
        conf->REUSE_TIME
    );
    V(
        chirp,
        conf->TIMEOUT <= conf->REUSE_TIME,
        "Config: timeout must be <= reuse time. (%f, %f)",
        conf->TIMEOUT,
        conf->REUSE_TIME
    );
    if(conf->FLOW_CONTROL) {
        V(
            chirp,
            conf->MAX_HANDLERS >= 16,
            "Config: if flow control is on max_handlers must be >= 16."
        );
    } else {
        V(
            chirp,
            conf->MAX_HANDLERS >= 1,
            "Config: max_handlers must be >= 1."
        );
    }
    if(conf->ACKNOWLEDGE == 0) {
        V(
            chirp,
            conf->RETRIES != 0,
            "Config: if acknowledge is disabled retries has to be 0."
        );
        V(
            chirp,
            conf->FLOW_CONTROL != 0,
            "Config: if acknowledge is disabled flow-control has to be 0."
        );
    }
    V(
        chirp,
        conf->MAX_HANDLERS <= 32,
        "Config: max_handlers must be <= 1."
    );
    V(
        chirp,
        conf->BUFFER_SIZE >= CH_MIN_BUFFER_SIZE || conf->BUFFER_SIZE == 0,
        "Config: buffer size must be > %d (%d)",
        CH_MIN_BUFFER_SIZE,
        conf->BUFFER_SIZE
    );
    V(
        chirp,
        conf->BUFFER_SIZE >= sizeof(ch_message_t) || conf->BUFFER_SIZE == 0,
        "Config: buffer size must be > %d (%d)",
        (int) sizeof(ch_message_t),
        conf->BUFFER_SIZE
    );
    V(
        chirp,
        conf->BUFFER_SIZE >= sizeof(
            ch_rd_handshake_t
        ) || conf->BUFFER_SIZE == 0,
        "Config: buffer size must be > %d (%d)",
        (int) sizeof(ch_rd_handshake_t),
        conf->BUFFER_SIZE
    );
    return CH_SUCCESS;
}

// .. c:function::
CH_EXPORT
ch_error_t
ch_chirp_close_ts(ch_chirp_t* chirp)
//    :noindex:
//
//    see: :c:func:`ch_chirp_close_ts`
//
//    This function is thread-safe.
//
// .. code-block:: cpp
//
{
    char chirp_closed = 0;
    ch_chirp_int_t* ichirp;
    if(chirp == NULL || chirp->_init != CH_CHIRP_MAGIC) {
        fprintf(
            stderr,
            "%s:%d Fatal: chirp is not initialzed. ch_chirp_t:%p\n",
            __FILE__,
            __LINE__,
            (void*) chirp
        );
        return CH_UNINIT; // NOCOV  TODO can be tested
    }
    A(chirp->_init == CH_CHIRP_MAGIC, "Not a ch_chirp_t*");
    if(chirp->_ != NULL) {
        ichirp = chirp->_;
        if(ichirp->flags & CH_CHIRP_CLOSED)
            chirp_closed = 1;
    } else
        chirp_closed = 1;
    if(chirp_closed) {
        fprintf(
            stderr,
            "%s:%d Fatal: chirp is already closed. ch_chirp_t:%p\n",
            __FILE__,
            __LINE__,
            (void*) chirp
        );
        return CH_FATAL;
    }
    if(ichirp->flags & CH_CHIRP_CLOSING) {
        E(
            chirp,
            "Close already in progress. ch_chirp_t:%p",
            (void*) chirp
        );
        return CH_IN_PRORESS;
    }
    ichirp->flags |= CH_CHIRP_CLOSING;
    ichirp->close.data = chirp;
    L(chirp, "Closing chirp via callback. ch_chirp_t:%p", (void*) chirp);
    if(uv_async_send(&ichirp->close) < 0) {
        E(
            chirp,
            "Could not call close callback. ch_chirp_t:%p",
            (void*) chirp
        );
        return CH_UV_ERROR; // NOCOV only breaking things will trigger this
    }
    return CH_SUCCESS;
}

// .. c:function::
CH_EXPORT
void
ch_chirp_config_init(ch_config_t* config)
//    :noindex:
//
//    see: :c:func:`ch_chirp_close_cb`
//
// .. code-block:: cpp
//
{
    *config = _ch_config_defaults;
}

// .. c:function::
CH_EXPORT
ch_identity_t
ch_chirp_get_identity(ch_chirp_t* chirp)
//    :noindex:
//
//    see: :c:func:`ch_chirp_get_identity`
//
// .. code-block:: cpp
//
{
    A(chirp->_init == CH_CHIRP_MAGIC, "Not a ch_chirp_t*");
    ch_identity_t id;
    memcpy(id.data, chirp->_->identity, sizeof(id.data));
    return id;
}

// .. c:function::
CH_EXPORT
uv_loop_t*
ch_chirp_get_loop(ch_chirp_t* chirp)
//    :noindex:
//
//    see: :c:func:`ch_chirp_get_loop`
//
// .. code-block:: cpp
//
{
    A(chirp->_init == CH_CHIRP_MAGIC, "Not a ch_chirp_t*");
    return chirp->_->loop;
}

// .. c:function::
CH_EXPORT
ch_error_t
ch_chirp_init(
        ch_chirp_t* chirp,
        const ch_config_t* config,
        uv_loop_t* loop,
        ch_start_cb_t start,
        ch_done_cb_t done,
        ch_log_cb_t log_cb
)
//    :noindex:
//
//    see: :c:func:`ch_chirp_init`
//
// .. code-block:: cpp
//
{
    int tmp_err;
    uv_mutex_lock(&_ch_chirp_init_lock);
    chirp->_done_cb = done;
    memset(chirp, 0, sizeof(ch_chirp_t));
    chirp->_init            = CH_CHIRP_MAGIC;
    ch_chirp_int_t* ichirp  = ch_alloc(sizeof(ch_chirp_int_t));
    if(!ichirp) {
        fprintf(
            stderr,
            "%s:%d Fatal: Could not allocate memory for chirp. ch_chirp_t:%p\n",
            __FILE__,
            __LINE__,
            (void*) chirp
        );
        uv_mutex_unlock(&_ch_chirp_init_lock);
        return CH_ENOMEM;
    }
    memset(ichirp, 0, sizeof(ch_chirp_int_t));
    ichirp->config          = *config;
    ichirp->public_port     = config->PORT;
    ichirp->loop            = loop;
    ichirp->start_cb        = start;
    ch_config_t* tmp_conf   = &ichirp->config;
    ch_protocol_t* protocol = &ichirp->protocol;
    ch_encryption_t* enc    = &ichirp->encryption;
    chirp->_                = ichirp;
    if(log_cb != NULL)
        ch_chirp_register_log_cb(chirp, log_cb);
    tmp_err = _ch_chirp_verify_cfg(chirp);
    if(tmp_err != CH_SUCCESS) {
        chirp->_init = 0;
        uv_mutex_unlock(&_ch_chirp_init_lock);
        return tmp_err;
    }

    srand((unsigned int) time(NULL));
    unsigned int i = 0;
    while(
            i < (sizeof(tmp_conf->IDENTITY) - 1) &&
            tmp_conf->IDENTITY[i] == 0
    ) i += 1;
    if(tmp_conf->IDENTITY[i] == 0)
        ch_random_ints_as_bytes(ichirp->identity, sizeof(ichirp->identity));
    else
        *ichirp->identity = *tmp_conf->IDENTITY;


    if(uv_async_init(loop, &ichirp->close, _ch_chirp_close_async_cb) < 0) {
        E(
            chirp,
            "Could not initialize close callback. ch_chirp_t:%p",
            (void*) chirp
        );
        ch_free(ichirp);
        chirp->_init = 0;
        uv_mutex_unlock(&_ch_chirp_init_lock);
        return CH_UV_ERROR; // NOCOV
    }
    if(uv_async_init(loop, &chirp->_done, _ch_chirp_done) < 0) {
        E(
            chirp,
            "Could not initialize done handler. ch_chirp_t:%p",
            (void*) chirp
        );
        ch_free(ichirp);
        chirp->_init = 0;
        uv_mutex_unlock(&_ch_chirp_init_lock);
        return CH_UV_ERROR; // NOCOV
    }
    chirp->_done.data = chirp;
    if(uv_async_init(loop, &ichirp->start, _ch_chirp_start) < 0) {
        E(
            chirp,
            "Could not initialize done handler. ch_chirp_t:%p",
            (void*) chirp
        );
        ch_free(ichirp);
        chirp->_init = 0;
        uv_mutex_unlock(&_ch_chirp_init_lock);
        return CH_UV_ERROR; // NOCOV
    }
    ichirp->start.data = chirp;
    if(uv_async_init(loop, &ichirp->send_ts, _ch_wr_send_ts_cb) < 0) {
        E(
            chirp,
            "Could not initialize send_ts handler. ch_chirp_t:%p",
            (void*) chirp
        );
        ch_free(ichirp);
        chirp->_init = 0;
        uv_mutex_unlock(&_ch_chirp_init_lock);
        return CH_UV_ERROR; // NOCOV
    }
    ichirp->send_ts.data = chirp;
    uv_mutex_init(&ichirp->send_ts_queue_lock);

    ch_pr_init(chirp, protocol);
    tmp_err = ch_pr_start(protocol);
    if(tmp_err != CH_SUCCESS) {
        E(
            chirp,
            "Could not start protocol: %d. ch_chirp_t:%p",
            tmp_err,
            (void*) chirp
        );
        ch_free(ichirp);
        chirp->_init = 0;
        uv_mutex_unlock(&_ch_chirp_init_lock);
        return tmp_err;
    }
    if(!ichirp->config.DISABLE_ENCRYPTION) {
        ch_en_init(chirp, enc);
        tmp_err = ch_en_start(enc);
        if(tmp_err != CH_SUCCESS) {
#       ifndef NDEBUG
                ERR_print_errors_fp(stderr);
#       endif
            E(
                chirp,
                "Could not start encryption: %d. ch_chirp_t:%p",
                tmp_err,
                (void*) chirp
            );
            ch_free(ichirp);
            chirp->_init = 0;
            uv_mutex_unlock(&_ch_chirp_init_lock);
            return tmp_err;
        }
    }
#   ifndef NDEBUG
    char id_str[33];
    ch_bytes_to_hex(
        ichirp->identity,
        sizeof(ichirp->identity),
        id_str,
        sizeof(id_str)
    );
    L(
        chirp,
        "Chirp initialized id: %s. ch_chirp_t:%p, uv_loop_t:%p",
        id_str,
        (void*) chirp,
        (void*) loop
    );
#   endif
    _ch_chirp_init_signals(chirp);
    uv_async_send(&ichirp->start);
    uv_mutex_unlock(&_ch_chirp_init_lock);
    return CH_SUCCESS;
}

// .. c:function::
void
ch_chirp_message_finish(
        ch_chirp_t* chirp,
        ch_message_t* msg,
        int status,
        float load
)
//    :noindex:
//
//    see: :c:func:`ch_chirp_message_finish`
//
// .. code-block:: cpp
//
{
    ch_chirp_int_t* ichirp = chirp->_;
    ch_message_t* base_msg = NULL;
#   ifdef NDEBUG
        if(sglib_ch_message_dest_t_delete_if_member(
            &ichirp->message_queue,
            msg,
            &base_msg
        ) == 0) {
            E(
                chirp,
                "Message queue inconstant. ch_chirp_t:%p ch_message_t:%p",
                (void*) chirp,
                (void*) msg
            );
        }
#   else
        base_msg = sglib_ch_message_dest_t_find_member(
            ichirp->message_queue,
            msg
        );
        A(
            base_msg != NULL,
            "On finish there must be a message in queue. "
            "ch_chirp_t:%p ch_message_t:%p",
            (void*) chirp,
            (void*) msg
        );
        A(
            base_msg == msg,
            "Message queue inconstant. ch_chirp_t:%p ch_message_t:%p",
            (void*) chirp,
            (void*) msg
        );
        sglib_ch_message_dest_t_delete(
            &ichirp->message_queue,
            msg
        );
#   endif
    ch_message_t* next = msg;
    CH_MQ_DEQUEUE(next);
    if(next != NULL) {
        L(
            chirp,
            "Dequeued message. ch_chirp_t:%p, ch_message_t:%p",
            (void*) chirp,
            (void*) msg
        );
        next->_flags &= ~CH_MSG_QUEUED;
        if(msg->_flags & CH_MSG_USER) {
            ch_chirp_send(chirp, next, next->_send_cb);
        } else {
            ch_wr_send(next->_conn, next);
        }
    }
    L(
        chirp,
        "Message finished. ch_chirp_t:%p, ch_message_t:%p",
        (void*) chirp,
        (void*) msg
    );
    msg->_flags &= ~CH_MSG_USED;
    if(msg->_send_cb != NULL) {
        /* The user may free the message in the cb */
        ch_send_cb_t cb = msg->_send_cb;
        msg->_send_cb = NULL;
        cb(msg, status, load);
    }
}

// .. c:function::
CH_EXPORT
ch_error_t
ch_chirp_run(
        const ch_config_t* config,
        ch_chirp_t** chirp_out,
        ch_start_cb_t start,
        ch_log_cb_t log
)
//    :noindex:
//
//    see: :c:func:`ch_chirp_run`
//
// .. code-block:: cpp
//
{
    ch_chirp_t chirp;
    uv_loop_t  loop;
    ch_error_t tmp_err;
    if(chirp_out == NULL) {
        return CH_UNINIT; // NOCOV  TODO can be tested
    }
    *chirp_out = NULL;

    tmp_err = ch_uv_error_map(ch_loop_init(&loop));
    chirp._log = NULL; /* Bootstrap order problem. E checks _log but
                        * ch_chirp_init() will initialize it. */
    if(tmp_err != CH_SUCCESS) {
        E(
            (&chirp),
            "Could not init loop: %d. uv_loop_t:%p",
            tmp_err,
            (void*) &loop
        );
        return tmp_err;  // NOCOV this can only fail with access error
    }
    tmp_err = ch_chirp_init(&chirp, config, &loop, start, NULL, log);
    if(tmp_err != CH_SUCCESS) {
        E(
            (&chirp),
            "Could not init chirp: %d ch_chirp_t:%p",
            tmp_err,
            (void*) &chirp
        );
        return tmp_err;  // NOCOV covered in ch_chirp_init tests
    }
    chirp._->flags |= CH_CHIRP_AUTO_STOP;
    L(
        (&chirp),
        "UV-Loop run by chirp. ch_chirp_t:%p, uv_loop_t:%p",
        (void*) &chirp,
        (void*) &loop
    );
    /* This works and is not TOO bad because the function blocks. */
    // cppcheck-suppress autoVariables
    *chirp_out = &chirp;
    tmp_err = ch_run(&loop);
    *chirp_out = NULL;
    if(tmp_err != 0) {
        E(
            (&chirp),
            "uv_run returned with error: %d, uv_loop_t:%p",
            tmp_err,
            (void*) &loop
        );
        return tmp_err; // NOCOV only breaking things will trigger this
    }
    if(ch_loop_close(&loop)) {
        return CH_UV_ERROR; // NOCOV only breaking things will trigger this
    }
    return CH_SUCCESS;
}

// .. c:function::
CH_EXPORT
void
ch_chirp_set_auto_stop_loop(ch_chirp_t* chirp)
//    :noindex:
//
//    see: :c:func:`ch_chirp_set_auto_stop_loop`
//
//    This function is thread-safe
//
// .. code-block:: cpp
//
{
    A(chirp->_init == CH_CHIRP_MAGIC, "Not a ch_chirp_t*");
    chirp->_->flags |= CH_CHIRP_AUTO_STOP;
}

// .. c:function::
CH_EXPORT
int
ch_loop_init(uv_loop_t* loop)
//    :noindex:
//
//    see: :c:func:`ch_loop_init`
//
// .. code-block:: cpp
//
{
    int tmp_err;
    uv_mutex_lock(&_ch_chirp_init_lock);
    tmp_err = uv_loop_init(loop);
    uv_mutex_unlock(&_ch_chirp_init_lock);
    return tmp_err;
}

// .. c:function::
CH_EXPORT
ch_error_t
ch_libchirp_cleanup(void)
//    :noindex:
//
//    see: :c:func:`ch_libchirp_cleanup`
//
// .. code-block:: cpp
//
{
    uv_mutex_destroy(&_ch_chirp_init_lock);
    return ch_en_openssl_cleanup();
}

// .. c:function::
CH_EXPORT
ch_error_t
ch_libchirp_init(void)
//    :noindex:
//
//    see: :c:func:`ch_libchirp_init`
//
// .. code-block:: cpp
//
{
    uv_mutex_init(&_ch_chirp_init_lock);
    return ch_en_openssl_init();
}

// .. c:function::
CH_EXPORT
void
ch_chirp_register_recv_cb(ch_chirp_t* chirp, ch_recv_cb_t recv_cb)
//    :noindex:
//
//    see: :c:func:`ch_chirp_register_recv_cb`
//
// .. code-block:: cpp
//
{
    ch_chirp_int_t* ichirp = chirp->_;
    ichirp->recv_cb = recv_cb;
}
