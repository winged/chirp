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

// System includes
// ===============
//
// .. code-block:: cpp
//
#include <uv.h>


// Typedefs
// ========
//
// .. code-block:: cpp

typedef char ch_buf; // Used to show it is not a c-string but a buffer

// Windows compatiblity
// ====================
//
// .. code-block:: cpp

#ifdef _WIN32
#   if defined(_MSC_VER) && _MSC_VER < 1600
#       include <stdint-msvc2008.h>
#       define ch_inline __inline
#   else // _MSC_VER
#       include <stdint.h>
#       define ch_inline inline
#   endif // _MSC_VER
#else
#   include <stdint.h>
#   define ch_inline inline
#endif

#endif //ch_libchirp_common_h
