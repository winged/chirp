// ========
// Libchirp
// ========
// 
// .. code-block:: cpp

#ifndef ch_libchirp_h
#define ch_libchirp_h

#include "libchirp/message.h"
#include "libchirp/chirp.h"
#include "libchirp/encryption.h"

// .. c:var:: extern char* ch_version
//
//    Version of chirp.
//
// .. code-block:: cpp
//
extern char* ch_version;

// .. c:function::
void
ch_set_alloc_funcs(
        ch_alloc_cb_t alloc,
        ch_realloc_cb_t realloc,
        ch_free_cb_t free
);
//
//    Set allocation functions.
//
//    .. note::
//
//       The user can change the functions multiple times. The user has to
//       ensure consistency of allocation/free pairs.
//
//    :param ch_alloc_cb_t alloc: Memory allocation callback. Has the same
//                                signature as malloc
//    :param ch_realloc_cb_t realloc: Memory reallocation callback. Has the same
//                                signature as realloc
//    :param ch_free_cb_t free: Callback to free memory. Has the same
//                                signature as free
//
// .. code-block:: cpp
//

#endif //ch_libchirp_h
