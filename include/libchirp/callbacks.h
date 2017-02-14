// ===============
// Chirp Callbacks
// ===============
// 
// If you are on embedded platform you have to set the memory functions of
// chirp, libuv and openssl.
//
// * ch_set_allocators
// * uv_replace_allocator_
// * CRYPTO_set_mem_functions_
//
// .. _uv_replace_allocator: http://docs.libuv.org/en/v1.x/misc.html
// .. _CRYPTO_set_mem_functions: https://www.openssl.org/docs/man1.1.0/crypto/OPENSSL_malloc.html
//
// .. code-block:: cpp

#ifndef ch_libchirp_callbacks_h
#define ch_libchirp_callbacks_h

#include <stdlib.h>

// .. c:type:: ch_alloc_cb_t
//
//    Callback used by chirp to request memory.
//
//    .. c:member:: size_t size
//
//       The size to allocate
//
// .. code-block:: cpp
//
typedef void* (*ch_alloc_cb_t)(size_t size);

// .. c:type:: ch_free_cb_t
//
//    Callback used by chirp to free memory.
//
// .. code-block:: cpp
//
typedef void (*ch_free_cb_t)(void* buf);

// .. c:type:: ch_log_cb_t
//
//    Logging callback
//
//    .. c:member:: char msg[]
//
//       The message to log
//
//    .. c:member:: char error
//
//       The message is a error
//
// .. code-block:: cpp

typedef void (*ch_log_cb_t)(char msg[], char error);

// .. c:type:: ch_realloc_cb_t
//
//    Callback used by chirp to request memory reallocation.
//
//    .. c:member:: void* buf
//
//       The Buffer to reallocate
//
//    .. c:member:: size_t new_size
//
//       The requested new size
//
// .. code-block:: cpp
//
typedef void (*ch_send_cb_t)(int status, float load);

// .. c:type:: ch_send_cb_t
//
//    Called by chirp when message is sent and can be freed.
//
//    .. c:member:: int status
//
//       Is CH_SUCCESS, CH_TIMEOUT, TODO list errors
//
//    .. c:member:: float load
//
//       The load of the remote peer. Range 0.0 - 1.0. 1.0 means all remote
//       handlers are blocked. Use this for load balancing between multiple
//       nodes. If flow control is not active or the remote node is completely
//       blocked, you are likely to see timeouts on high load.
//
// .. code-block:: cpp
//
typedef void* (*ch_realloc_cb_t)(void* buf, size_t new_size);

#endif //ch_libchirp_callbacks_h
