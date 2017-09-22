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

// rbtree Prototypes
// =================
//
// .. code-block:: cpp
//
rb_bind_impl_m(ch_rm, ch_remote_t)

// Definitions
// ===========
//
// .. code-block:: cpp

static
ch_inline
void
_ch_rm_init(
        ch_chirp_t* chirp,
        ch_remote_t* remote
)
//
//    Initialize remote
//
// .. code-block:: cpp
//
{
    memset(remote, 0, sizeof(ch_remote_t));
    ch_rm_node_init(remote);
    remote->load  = -1;
    remote->chirp = chirp;
}

// .. c:function::
void
ch_rm_init_from_msg(
        ch_chirp_t* chirp,
        ch_remote_t* remote,
        ch_message_t* msg
)
//    :noindex:
//
//    see: :c:func:`ch_rm_init_from_msg`
//
// .. code-block:: cpp
//
{
    _ch_rm_init(chirp, remote);
    remote->ip_protocol = msg->ip_protocol;
    remote->port        = msg->port;
    memcpy(
        &remote->address,
        &msg->address,
        CH_IP_ADDR_SIZE
    );
    remote->conn = NULL;
}

// .. c:function::
void
ch_rm_init_from_conn(
        ch_chirp_t* chirp,
        ch_remote_t* remote,
        ch_connection_t* conn
)
//    :noindex:
//
//    see: :c:func:`ch_rm_init_from_conn`
//
// .. code-block:: cpp
//
{
    _ch_rm_init(chirp, remote);
    remote->ip_protocol = conn->ip_protocol;
    remote->port        = conn->port;
    memcpy(
        &remote->address,
        &conn->address,
        CH_IP_ADDR_SIZE
    );
    remote->conn = NULL;
}
