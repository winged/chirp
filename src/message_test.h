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
void
ch_test_gen_message(ch_buf* data);
//
//    Generate a message (ch_message_t*). The memoy allocated, tracked and
//    freed by quickcheck.
//
//    :param ch_buf* data: Out parameter set to :c:type:`ch_qc_mem_track_t`\*
//                         which contains the requested ch_message_t* in
//                         its data field.

// Definitions
// ===========


#endif //ch_message_test_h
