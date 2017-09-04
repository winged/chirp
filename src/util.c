// ====
// Util
// ====
//
// Common utility functions.
//
// .. code-block:: cpp

// Project includes
// ================
//
// .. code-block:: cpp
//
#include "util.h"

// Declarations
// ============

static
int
_ch_always_encrypt = 0;

static
ch_free_cb_t
_ch_free_cb = free;

static
ch_alloc_cb_t
_ch_alloc_cb = malloc;

static
ch_realloc_cb_t
_ch_realloc_cb = realloc;

// Definitions
// ===========

// .. c:function::
void*
ch_alloc(size_t size)
//    :noindex:
//
//    see: :c:func:`ch_alloc`
//
// .. code-block:: cpp
//
{
    void* buf = _ch_alloc_cb(size);
    assert(buf); /* Assert memory (do not rely on this, implement it
                    robust: be graceful and return error to user) */
    return buf;
}

// .. c:function::
void
ch_chirp_set_always_encrypt()
//    :noindex:
//
//    see: :c:func:`ch_chirp_set_always_encrypt`
//
// .. code-block:: cpp
//
{
    _ch_always_encrypt = 1;
}

// .. c:function::
void
ch_free(void* buf)
//    :noindex:
//
//    see: :c:func:`ch_free`
//
// .. code-block:: cpp
//
{
    _ch_free_cb(buf);
}

// .. c:function::
int
ch_is_local_addr(ch_text_address_t* addr)
//    :noindex:
//
//    see: :c:func:`ch_is_local_addr`
//
// .. code-block:: cpp
//
{
    if(_ch_always_encrypt)
        return 0;
    else {
        return (
            strncmp("::1", addr->data, sizeof(ch_text_address_t)) == 0 ||
            strncmp("127.0.0.1", addr->data, sizeof(ch_text_address_t)) == 0
        );
    }
}

// .. c:function::
void*
ch_realloc(
        void*  buf,
        size_t size
)
//    :noindex:
//
//    see: :c:func:`ch_realloc`
//
// .. code-block:: cpp
//
{
    void* rbuf = _ch_realloc_cb(buf, size);
    assert(rbuf); /* Assert memory (do not rely on this, implement it
                    robust: be graceful and return error to user) */
    return rbuf;
}

// .. c:function::
CH_EXPORT
void
ch_set_alloc_funcs(
        ch_alloc_cb_t alloc,
        ch_realloc_cb_t realloc,
        ch_free_cb_t free
)
//    :noindex:
//
//    see: :c:func:`ch_set_alloc_funcs`
//
// .. code-block:: cpp
//
{
    _ch_alloc_cb = alloc;
    _ch_realloc_cb = realloc;
    _ch_free_cb = free;
}
