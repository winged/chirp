// ===================
// Message test Header
// ===================
//
// Generate messages for tests
//
// .. code-block:: cpp
//
#ifndef ch_message_test_h
#define ch_message_test_h

// Project includes
// ================
//
// .. code-block:: cpp
//
#include "common.h"
#include "message.h"
#include "buffer.h"

// Declarations
// ============

// .. c:function::
ch_message_t*
ch_test_gen_message(struct ch_chirp_s* chirp);
//
//    Generate a message (ch_message_t*). The memoy allocated, tracked and
//    freed by quickcheck.
//
//    :param struct ch_chirp_s* chirp: Pointer to chirp instance
//
//    :rtype: ch_message_t*

// Definitions
// ===========


#endif //ch_message_test_h
