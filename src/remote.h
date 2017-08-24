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

// Declarations
// ============

// .. c:type:: ch_remote_t
//
//    ch_remote_t represents remote node and is a dictionary used to lookup
//    remotes.
//
//    .. c:member:: uint8_t ip_protocol
//
// .. code-block:: cpp
//
struct ch_remote_s {
    uint8_t          ip_protocol;
    uint8_t          address[CH_IP_ADDR_SIZE];
    int32_t          port;
    ch_connection_t* conn;
    char             color_field;
    ch_remote_t*     parent;
    ch_remote_t*     left;
    ch_remote_t*     right;
};

// .. c:function::
void
ch_rm_init(ch_remote_t* remote, ch_message_t* msg);
//
//    Initialize the remote data-structure from a message.
//
//    :param ch_remote_t* remote: Remote to initialize
//    :param ch_message_t* msg:   Message to initialize from

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