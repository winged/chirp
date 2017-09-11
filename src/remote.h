// =============
// Remote header
// =============
//
// ch_remote_t represents a remote node.
//
// .. code-block:: cpp
//
#ifndef ch_remote_h
#define ch_remote_h

// Project includes
// ================
//
// .. code-block:: cpp
//
#include "common.h"
#include "connection.h"
#include "rbtree.h"

// Declarations
// ============

// .. c:type:: ch_remote_t
//
//    ch_remote_t represents remote node and is a dictionary used to lookup
//    remotes.
//
//    .. c:member:: uint8_t ip_protocol
//
//       What IP protocol (IPv4 or IPv6) shall be used for connections.
//
//    .. c:member:: uint8_t[16] address
//
//       IPv4/6 address of the sender if the message was received.  IPv4/6
//       address of the recipient if the message is going to be sent.
//
//    .. c:member:: int32_t port
//
//       The port that shall be used for connections.
//
//    .. c:member:: uint8_t receipt[CH_ID_SIZE]
//
//       The last receipt for this remote. Used to detect duplicate messages.
//
//    .. c:member:: ch_connection_t* conn
//
//       The active connection to this remote. Can be NULL. Callbacks always
//       have to check if the connection is NULL. The code that sets the
//       connection to NULL has to initiate retry and notify the user. So
//       callbacks can safely abort if conn is NULL.
//
//    .. c:member:: ch_message_t* msg_queue
//
//       The message queue, the head of this queue is the active message.
//
//    .. c:member:: ch_chirp_t* chirp
//
//       Pointer to the chirp object. See: :c:type:`ch_chirp_t`.
//
//    .. c:member:: float load
//
//       Last reported load of the remote
//
//    .. c:member:: char color
//
//       rbtree member
//
//    .. c:member:: ch_remote_t* left
//
//       rbtree member
//
//    .. c:member:: ch_remote_t* right
//
//       rbtree member
//
//    .. c:member:: ch_remote_t* parent
//
//       rbtree member
//
// .. code-block:: cpp
//
struct ch_remote_s {
    uint8_t          ip_protocol;
    uint8_t          address[CH_IP_ADDR_SIZE];
    int32_t          port;
    uint8_t          receipt[CH_ID_SIZE];
    ch_connection_t* conn;
    ch_message_t*    msg_queue;
    ch_chirp_t*      chirp;
    float            load;
    char             color;
    ch_remote_t*     parent;
    ch_remote_t*     left;
    ch_remote_t*     right;
};

// rbtree Prototypes
// ------------------
//
// .. code-block:: cpp
//
#define ch_rm_cmp_m(x,y) ch_remote_cmp(x, y)

// .. code-block:: cpp
//
rb_bind_decl_m(ch_rm, ch_remote_t)

// .. c:function::
void
ch_rm_init_from_msg(
        ch_chirp_t* chirp,
        ch_remote_t* remote,
        ch_message_t* msg
);
//
//    Initialize the remote data-structure from a message.
//
//    :param ch_remote_t* remote: Remote to initialize
//    :param ch_message_t* msg:   Message to initialize from

// .. c:function::
void
ch_rm_init_from_conn(
        ch_chirp_t* chirp,
        ch_remote_t* remote,
        ch_connection_t* conn
);
//
//    Initialize the remote data-structure from a connection.
//
//    :param ch_remote_t* remote: Remote to initialize
//    :param ch_connection_t*:   Connection to initialize from

// Definitions
// ===========

// .. c:function::
static
ch_inline
int
ch_remote_cmp(ch_remote_t* x, ch_remote_t* y)
//
//    Compare operator for connections.
//
//    :param ch_remote_t* x: First remote instance to compare
//    :param ch_remote_t* y: Second remote instance to compare
//
//    :return: the comparision between
//                 - the IP protocols, if they are not the same, or
//                 - the addresses, if they are not the same, or
//                 - the ports
//    :rtype: int
//
// .. code-block:: cpp
//
{
    if(x->ip_protocol != y->ip_protocol) {
        return x->ip_protocol - y->ip_protocol;
    } else {
        int tmp_cmp = memcmp(
            x->address,
            y->address,
            x->ip_protocol == CH_IPV6 ? CH_IP_ADDR_SIZE : CH_IP4_ADDR_SIZE
        );
        if(tmp_cmp != 0) {
            return tmp_cmp;
        } else {
            return x->port - y->port;
        }
    }
}

#endif // ch_remote_h
