// ====
// Util
// ====
//
// .. code-block:: cpp
//
#include "util.h"

ch_free_cb_t _ch_free_cb = free;
ch_alloc_cb_t _ch_alloc_cb = malloc;
ch_realloc_cb_t _ch_realloc_cb = realloc;

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
    return _ch_alloc_cb(size);
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
    return _ch_realloc_cb(buf, size);
}

// .. c:function::
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
