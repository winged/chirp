// ==========
// Encryption
// ==========
//
// .. todo:: Document purpose
//

// Project includes
// ================
//
// .. code-block:: cpp
//
#include "encryption.h"
#include "chirp.h"

// System includes
// ===============
//
// .. code-block:: cpp
//
#include <openssl/crypto.h>
#include <openssl/engine.h>
#include <openssl/conf.h>
#include <openssl/err.h>

// Declarations
// ============

// .. c:var:: _ch_en_manual_openssl
//
//    The user will call ch_en_openssl_init() and ch_en_openssl_cleanup().
//    Defaults to 0.
//
// .. code-block:: cpp
//
static char _ch_en_manual_openssl = 0;

// .. c:var:: _ch_en_lock_count
//
//    The count of locks created for openssl.
//
// .. code-block:: cpp
//
static int _ch_en_lock_count = 0;

// .. c:var:: _ch_en_lock_list
//
//    List of locks provided for openssl. Openssl will tell us how many locks
//    it needs.
//
// .. code-block:: cpp
//
static uv_rwlock_t* _ch_en_lock_list = NULL;

// .. c:function::
static
void
_ch_en_locking_function(int mode, int n, const char *file, int line);
//
//    Called by openssl to lock a mutex.
//
//    :param int mode: Can be CRYPTO_LOCK or CRYPTO_UNLOCK
//    :param int n: The lock to lock/unlock
//    :param const char* file: File the function was called from (deubbing)
//    :param const char* line: Line the function was called from (deubbing)
//

// .. c:function::
static
unsigned long
_ch_en_thread_id_function(void);
//
//    Called by openssl to get the current thread it.
//

// Definitions
// ===========

// .. c:function::
static
void
_ch_en_locking_function(int mode, int n, const char *file, int line)
//    :noindex:
//
//    see: :c:func:`_ch_en_locking_function`
//
// .. code-block:: cpp
//
{
    (void)(file);
    (void)(line);
    if(mode & CRYPTO_LOCK) {
        if(mode & CRYPTO_WRITE)  // The user requested write
            uv_rwlock_wrlock(&_ch_en_lock_list[n]);
        else if(mode & CRYPTO_READ) // The user requested read
            uv_rwlock_rdlock(&_ch_en_lock_list[n]);
        else  // The user requested something bad, do a wrlock for safety
            uv_rwlock_wrlock(&_ch_en_lock_list[n]);
    } else {
        if(mode & CRYPTO_WRITE)
            uv_rwlock_wrunlock(&_ch_en_lock_list[n]);
        else if(mode & CRYPTO_READ)
            uv_rwlock_rdunlock(&_ch_en_lock_list[n]);
        else
            uv_rwlock_wrunlock(&_ch_en_lock_list[n]);
    }
}

// .. c:function::
ch_error_t
ch_en_openssl_init(void)
//    :noindex:
//
//    see: :c:func:`ch_en_openssl_init`
//
// .. code-block:: cpp
//
{
    SSL_library_init();
    OpenSSL_add_all_algorithms();
    SSL_load_error_strings();
    OPENSSL_config("chirp");

    return ch_en_openssl_threading_setup();
}

// .. c:function::
ch_error_t
ch_en_openssl_cleanup(void)
//    :noindex:
//
//    see: :c:func:`ch_en_openssl_cleanup`
//
// .. code-block:: cpp
//
{
    if(_ch_en_manual_openssl) {
        return CH_SUCCESS;
    }

    FIPS_mode_set(0);
    ENGINE_cleanup();
    CONF_modules_unload(1);
    ERR_free_strings();
    CONF_modules_free();
    EVP_cleanup();
    CRYPTO_cleanup_all_ex_data();
    CRYPTO_THREADID id;
    CRYPTO_THREADID_current(&id);
    ERR_remove_thread_state(&id);
    ASN1_STRING_TABLE_cleanup();

    return ch_en_openssl_threading_cleanup();
}

// .. c:function::
ch_error_t
ch_en_openssl_threading_cleanup(void)
//    :noindex:
//
//    see: :c:func:`ch_en_openssl_threading_cleanup`
//
// .. code-block:: cpp
//
{
    if(!_ch_en_lock_list) {
        fprintf(
            stderr,
            "%s:%d Fatal: Threading not setup.\n",
            __FILE__,
            __LINE__
        );
        return CH_VALUE_ERROR;
    }
    CRYPTO_set_id_callback(NULL);
    CRYPTO_set_locking_callback(NULL);
    for(int i = 0;  i < _ch_en_lock_count;  i++)
        uv_rwlock_destroy(&_ch_en_lock_list[i]);
    ch_free(_ch_en_lock_list);
    _ch_en_lock_list = NULL;
    return 0;
}


// .. c:function::
ch_error_t
ch_en_openssl_threading_setup(void)
//    :noindex:
//
//    see: :c:func:`ch_en_openssl_threading_setup`
//
// .. code-block:: cpp
//
{
    int lock_count = CRYPTO_num_locks();
    _ch_en_lock_list = ch_alloc(
        lock_count * sizeof(uv_rwlock_t)
    );
    if(!_ch_en_lock_list) {
        fprintf(
            stderr,
            "%s:%d Fatal: Could not allocate memory for locking.\n",
            __FILE__,
            __LINE__
        );
        return CH_ENOMEM;
    }
    _ch_en_lock_count = lock_count;
    for(int i = 0;  i < lock_count;  i++)
        uv_rwlock_init(&_ch_en_lock_list[i]);
    CRYPTO_set_id_callback(_ch_en_thread_id_function);
    CRYPTO_set_locking_callback(_ch_en_locking_function);
    return 0;
}

// .. c:function::
void
ch_en_set_manual_openssl_init(void)
//    :noindex:
//
//    see: :c:func:`ch_en_set_manual_openssl_init`
//
// .. code-block:: cpp
//
{
    _ch_en_manual_openssl = 1;
}

// .. c:function::
ch_error_t
ch_en_start(ch_encryption_t* enc)
//    :noindex:
//
//    see: :c:func:`ch_en_start`
//
// .. code-block:: cpp
//
{
    ch_chirp_t* chirp = enc->chirp;
    A(chirp->_init == CH_CHIRP_MAGIC, "Not a ch_chirp_t*");
    ch_chirp_int_t* ichirp = chirp->_;
    if(!_ch_en_manual_openssl) {
        int tmp_err;
        L(
            chirp,
            "Initializing the OpenSSL library. ch_chirp_t:%p",
            (void*) chirp
        );
        tmp_err = ch_en_openssl_init();
        if(tmp_err != CH_SUCCESS) {
            L(
                chirp,
                "Could not initialize the OpenSSL library. ch_chirp_t:%p",
                (void*) chirp
            );
            return tmp_err;
        }
    }
    const SSL_METHOD* method = TLSv1_2_method();
    if(method == NULL) {
        E(
            chirp,
            "Could not get the TLSv1_2_method. ch_chirp_t:%p",
            (void*) chirp
        );
        return CH_TLS_ERROR;
    }
    enc->ssl_ctx = SSL_CTX_new(method);
    if(enc->ssl_ctx == NULL) {
        E(
            chirp,
            "Could create the SSL_CTX. ch_chirp_t:%p",
            (void*) chirp
        );
        return CH_TLS_ERROR;
    }
    SSL_CTX_set_mode(
        enc->ssl_ctx,
        SSL_MODE_AUTO_RETRY |
        SSL_MODE_ENABLE_PARTIAL_WRITE
    );
    SSL_CTX_set_options(enc->ssl_ctx, SSL_OP_NO_COMPRESSION);
    SSL_CTX_set_verify(
            enc->ssl_ctx,
            SSL_VERIFY_PEER | SSL_VERIFY_FAIL_IF_NO_PEER_CERT,
            NULL
    );
    SSL_CTX_set_verify_depth(enc->ssl_ctx, 5);
    if(SSL_CTX_load_verify_locations(
                enc->ssl_ctx,
                ichirp->config.CERT_CHAIN_PEM,
                NULL
    ) != 1) {
        E(
            chirp,
            "Could not set the verification certificate "
            "%s. ch_chirp_t:%p",
            ichirp->config.CERT_CHAIN_PEM,
            (void*) chirp
        );
        SSL_CTX_free(enc->ssl_ctx);
        return CH_TLS_ERROR;
    }
    if(SSL_CTX_use_certificate_chain_file(
                enc->ssl_ctx,
                ichirp->config.CERT_CHAIN_PEM
    ) != 1) {
        E(
            chirp,
            "Could not set the certificate %s. ch_chirp_t:%p",
            ichirp->config.CERT_CHAIN_PEM,
            (void*) chirp
        );
        SSL_CTX_free(enc->ssl_ctx);
        return CH_TLS_ERROR;
    }
    if(SSL_CTX_use_PrivateKey_file(
                enc->ssl_ctx,
                ichirp->config.CERT_CHAIN_PEM,
                SSL_FILETYPE_PEM
    ) != 1) {
        E(
            chirp,
            "Could not set the private key %s. ch_chirp_t:%p",
            ichirp->config.CERT_CHAIN_PEM,
            (void*) chirp
        );
        SSL_CTX_free(enc->ssl_ctx);
        return CH_TLS_ERROR;
    }
    if(SSL_CTX_check_private_key(enc->ssl_ctx) != 1) {
        E(
            chirp,
            "Private key is not valid %s. ch_chirp_t:%p",
            ichirp->config.CERT_CHAIN_PEM,
            (void*) chirp
        );
        SSL_CTX_free(enc->ssl_ctx);
        return CH_TLS_ERROR;
    }
    DH *dh = NULL;
    FILE *paramfile;
    paramfile = fopen(ichirp->config.DH_PARAMS_PEM, "r");
    if(paramfile == NULL) {
        E(
            chirp,
            "Could not open the dh-params %s. ch_chirp_t:%p",
            ichirp->config.DH_PARAMS_PEM,
            (void*) chirp
        );
        SSL_CTX_free(enc->ssl_ctx);
        return CH_TLS_ERROR;
    }
    dh = PEM_read_DHparams(paramfile, NULL, NULL, NULL);
    fclose(paramfile);
    if(dh == NULL) {
        E(
            chirp,
            "Could not load the dh-params %s. ch_chirp_t:%p",
            ichirp->config.DH_PARAMS_PEM,
            (void*) chirp
        );
        SSL_CTX_free(enc->ssl_ctx);
        return CH_TLS_ERROR;
    }
    if(SSL_CTX_set_tmp_dh(enc->ssl_ctx, dh) != 1) {
        E(
            chirp,
            "Could not set the dh-params %s. ch_chirp_t:%p",
            ichirp->config.DH_PARAMS_PEM,
            (void*) chirp
        );
        ch_free(dh);
        SSL_CTX_free(enc->ssl_ctx);
        return CH_TLS_ERROR;
    }
    if(SSL_CTX_set_cipher_list(
            enc->ssl_ctx,
            "-ALL:"
            "DHE-DSS-AES256-GCM-SHA384:"
            "DHE-RSA-AES256-GCM-SHA384:"
            "DHE-RSA-AES256-SHA256:"
            "DHE-DSS-AES256-SHA256:"
    ) != 1) {
        E(
            chirp,
            "Could not set the cipher list. ch_chirp_t:%p",
            (void*) chirp
        );
        ch_free(dh);
        SSL_CTX_free(enc->ssl_ctx);
        return CH_TLS_ERROR;
    }
    L(
        chirp,
        "Created SSL context for chirp. ch_chirp_t:%p",
        (void*) chirp
    );
    ch_free(dh);
    return CH_SUCCESS;
}

// .. c:function::
ch_error_t
ch_en_stop(ch_encryption_t* enc)
//    :noindex:
//
//    see: :c:func:`ch_en_stop`
//
// .. code-block:: cpp
//
{
    ch_chirp_t* chirp = enc->chirp;
    A(chirp->_init == CH_CHIRP_MAGIC, "Not a ch_chirp_t*");
    SSL_CTX_free(enc->ssl_ctx);
    CRYPTO_THREADID id;
    CRYPTO_THREADID_current(&id);
    ERR_remove_thread_state(&id);
    return CH_SUCCESS;
}

// .. c:function::
static
unsigned long
_ch_en_thread_id_function(void)
//    :noindex:
//
//    see: :c:func:`_ch_en_thread_id_function`
//
// .. code-block:: cpp
//
{
    uv_thread_t self = uv_thread_self();
    return (unsigned long)self;
}
