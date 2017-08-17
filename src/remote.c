// ======
// Writer
// ======
//

// Project includes
// ================
//
// .. code-block:: cpp
//
#include "remote.h"

// Definitions
// ===========
//
// .. code-block:: cpp

// .. c:function::
void
ch_rm_init(ch_remote_t* remote, ch_message_t* msg)
//    :noindex:
//
//    see: :c:func:`ch_rm_init`
//
// .. code-block:: cpp
//
{
    (void)(msg);
    memset(remote, 0, sizeof(ch_remote_t));
    remote->ip_protocol = msg->ip_protocol;
    remote->port        = msg->port;
    memcpy(
        &remote->address,
        &msg->address,
        CH_IP_ADDR_SIZE
    );
    remote->conn = NULL;
}
