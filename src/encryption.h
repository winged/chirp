// =================
// Encryption Header
// =================
//
// .. code-block:: cpp

#ifndef ch_encryption_h
#define ch_encryption_h

#include "../include/chirp_obj.h"

#include <openssl/ssl.h>

// .. c:type:: ch_en_tls_ops_t
//
//    Represents tls operations
//
//    .. c:member:: CH_EN_OP_HANDSHAKE
//
//       Continue with handshake
//
//    .. c:member:: CH_EN_OP_READ
//
//       Read data from remote
//
//    .. c:member:: CH_EN_OP_WRITE
//
//       Write data to remote
//
//    .. c:member:: CH_EN_OP_SHUTDOWN
//
//       Continue with shutdown
//
// .. code-block:: cpp
//
typedef enum {
    CH_EN_OP_HANDSHAKE   = 0,
    CH_EN_OP_READ        = 1,
    CH_EN_OP_WRITE       = 2,
    CH_EN_OP_SHUTDOWN    = 3,
} ch_en_tls_ops_t;

// .. c:type:: ch_encryption_t
//
//    Encryption object.
//
//    .. c:member:: struct ch_chirp_s*
//
//       reference back to chirp
//
// .. code-block:: cpp

typedef struct ch_encryption_s {
    ch_chirp_t* chirp;
    SSL_CTX*    ssl_ctx;
} ch_encryption_t;

// .. c:function::
static
void
_cn_en_free(void* buf, const char* file, int line);
//
//    Free callback for OpenSSL.
//
//    :param void* buf: Buffer to free
//    :param const char* file: File the allocation came from
//    :param int line: The line the allocation came from

// .. c:function::
static
ch_inline
void
ch_en_init(ch_chirp_t* chirp, ch_encryption_t* enc)
//
//    Initialize the encryption struct.
//
//    :param ch_chirp_t* chirp: Chirp instance
//    :param ch_encryption_t* enc: Encryption to initialize
//
// .. code-block:: cpp
//
{
    memset(enc, 0, sizeof(ch_encryption_t));
    enc->chirp = chirp;
}

// .. c:function::
static
void*
_cn_en_malloc(size_t size, const char* file, int line);
//
//    Malloc callback for OpenSSL.
//
//    :param size_t size: Size to allocate
//    :param const char* file: File the allocation came from
//    :param int line: The line the allocation came from
//
// .. c:function::
static
void*
_cn_en_realloc(void* buf, size_t new_size, const char* file, int line);
//
//    Realloc callback for OpenSSL.
//
//    :param void* buf: Buffer to reallocate
//    :param size_t size: Size to allocate
//    :param const char* file: File the allocation came from
//    :param int line: The line the allocation came from

// .. c:function::
ch_error_t
ch_en_start(ch_encryption_t* enc);
//
//    Start the encryption
//
//    TODO params
//
// .. c:function::
ch_error_t
ch_en_stop(ch_encryption_t* enc);
//
//    Stop the encryption
//
//    TODO params
//
// .. code-block:: cpp
//
#endif //ch_encryption_h
