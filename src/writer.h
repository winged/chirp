// =============
// Writer Header
// =============
//
// Chirp protocol writer
//
// .. code-block:: cpp

#ifndef ch_writer_h
#define ch_writer_h

#include "../include/common.h"
#include "../include/callbacks.h"
#include "../include/message.h"

struct ch_connection_s;

typedef struct ch_writer_s {
    ch_send_cb_t    send_cb;
    uv_timer_t      send_timeout;
    uv_mutex_t      lock;
    ch_message_t*   msg;
    ch_ms_message_t net_msg;
} ch_writer_t;

// .. c:function::
static
ch_inline
void
ch_wr_free(ch_writer_t* writer)
//
//    Initialize the writer structure
//
//    :param ch_writer_t* writer: The writer
//
// .. code-block:: cpp
//
{
    uv_mutex_destroy(&writer->lock);
}

// .. c:function::
void
ch_wr_init(ch_writer_t* writer, struct ch_connection_s* conn);
//
//    Initialize the writer structure
//
//    :param ch_chirp_t* chirp: Chirp instance
//    :param ch_connection_t* conn: The connection
//

#endif //ch_writer_h
