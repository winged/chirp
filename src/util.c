// ====
// Util
// ====
//
// Common utility functions.
//

// Project includes
// ================
//
// .. code-block:: cpp

#include "util.h"
#include "rbtree.h"

// Declarations
// ============
//
// .. code-block:: cpp

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

// Debug alloc tracking
// ====================
//
// Since we can't use valgrind and rr at the same time and I needed to debug a
// leak with rr, I added this memory leak debugging code. Since we use rr, the
// pointer to the allocation is enough, we don't need any meta information.
//
// .. code-block:: cpp

#ifndef NDEBUG
    struct ch_alloc_track_s;
    typedef struct ch_alloc_track_s ch_alloc_track_t;
    struct ch_alloc_track_s {
        void*             buf;
        char              color;
        ch_alloc_track_t* parent;
        ch_alloc_track_t* left;
        ch_alloc_track_t* right;
    };

#   define _ch_at_cmp_m(x, y) rb_safe_cmp_m(x->buf, y->buf)

    rb_bind_m(_ch_at, ch_alloc_track_t)

    ch_alloc_track_t* _ch_alloc_tree;

// .. c:function::
    static
    ch_inline
    void*
    _ch_at_alloc(void* buf)
//
//    Track a memory allocation.
//
//    :param void* buf: Pointer to the buffer to track.
//
// .. code-block:: cpp
//
    {
        /* We do not track failed allocations */
        if(buf == NULL)
            return buf;
        ch_alloc_track_t* track = _ch_alloc_cb(sizeof(*track));
        assert(track);
        /* We treat a failure to track as an alloc failure. This code is here
         * if we ever need to use this in release mode (or just for
         * correctness). */
        if(track == NULL)
            return NULL;
        _ch_at_node_init(track);
        track->buf = buf;
        int ret = _ch_at_insert(&_ch_alloc_tree, track);
        assert(ret == 0);
        return buf;
    }

// .. c:function::
    void
    ch_at_cleanup(void)
//    :noindex:
//
//    see: :c:func:`ch_at_cleanup`
//
// .. code-block:: cpp
//
    {
        fprintf(stderr, "Leaked allocations: \n");
        while(_ch_alloc_tree != _ch_at_nil_ptr) {
            ch_alloc_track_t* item;
            item = _ch_alloc_tree;
            fprintf(stderr, "%p ", item->buf);
            _ch_at_delete_node(&_ch_alloc_tree, item);
            _ch_free_cb(item);
        }
        fprintf(stderr, "\n");
    }

// .. c:function::
    void
    ch_at_init(void)
//    :noindex:
//
//    see: :c:func:`ch_at_init`
//
// .. code-block:: cpp
//
    {
        _ch_at_tree_init(&_ch_alloc_tree);
    }

// .. c:function::
    static
    ch_inline
    void
    _ch_at_free(void* buf)
//
//    Track a freed memory allocation.
//
//    :param void* buf: Pointer to the buffer to track.
//
// .. code-block:: cpp
//
    {
        ch_alloc_track_t key;
        ch_alloc_track_t* track;
        key.buf = buf;
        int ret = _ch_at_delete(&_ch_alloc_tree, &key, &track);
        assert(ret == 0);
        _ch_free_cb(track);
    }

// .. c:function::
    static
    ch_inline
    void*
    _ch_at_realloc(void* buf, void* rbuf)
//
//    Track a memory reallocation.
//
//    :param void* buf: Pointer to the buffer that has been reallocated.
//    :param void* rbuf: Pointer to the new buffer.
//
// .. code-block:: cpp
//
    {
        /* We do not track failed reallocations */
        if(rbuf == NULL)
            return rbuf;
        ch_alloc_track_t key;
        ch_alloc_track_t* track;
        /* Shortcut if the allocator was able to extend the allocation */
        if(buf == rbuf)
            return rbuf;
        key.buf = buf;
        int ret = _ch_at_delete(&_ch_alloc_tree, &key, &track);
        assert(ret == 0);
        track->buf = rbuf;
        ret = _ch_at_insert(&_ch_alloc_tree, track);
        assert(ret == 0);
        return rbuf;
    }
#endif


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
#   ifndef NDEBUG
        return _ch_at_alloc(buf);
#   else
        return buf;
#   endif
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
#   ifndef NDEBUG
        _ch_at_free(buf);
#   endif
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
            strncmp("::1", addr->data, sizeof(addr->data)) == 0 ||
            strncmp("127.0.0.1", addr->data, sizeof(addr->data)) == 0
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
#   ifndef NDEBUG
        return _ch_at_realloc(buf, rbuf);
#   else
        return rbuf;
#   endif
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
