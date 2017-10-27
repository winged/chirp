// =============
// Common header
// =============
//
// .. code-block:: cpp
//
#ifndef ch_libchirp_common_h
#define ch_libchirp_common_h

// Project includes
// ================
//
// .. code-block:: cpp
//
#include "libchirp/const.h"
#include "libchirp/error.h"

// Library export
// ==============
//
// .. code-block:: cpp
//
#ifdef CH_BUILD
#   if defined __GNUC__ || __clang__
#       define CH_EXPORT __attribute__ ((visibility ("default")))
#   endif
#else // CH_BUILD
#   define CH_EXPORT
#endif

// System includes
// ===============
//
// .. code-block:: cpp
//
#include <uv.h>
#include <stdio.h>
#include <stdint.h>


// Typedefs
// ========
//
// .. code-block:: cpp

typedef char ch_buf; /* Used to show it is not a c-string but a buffer. */

// .. c:type:: ch_text_address_t
//
//    Type to be used with :c:func:`ch_msg_get_address`. Used for the textual
//    representation of the IP-address.
//
// .. code-block:: cpp
//
typedef struct ch_text_address_s {
    char data[INET6_ADDRSTRLEN];
} ch_text_address_t;


// Forward declarations
// =====================
//
// .. code-block:: cpp

struct ch_chirp_s;
typedef struct ch_chirp_s ch_chirp_t;
struct ch_config_s;
typedef struct ch_config_s ch_config_t;
struct ch_message_s;
typedef struct ch_message_s ch_message_t;

#endif //ch_libchirp_common_h
