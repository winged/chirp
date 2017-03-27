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
    assert(buf); // Assert we got memory
    return buf;
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
    assert(rbuf); // Assert we got memory
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
