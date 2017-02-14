// ===============
// Protocol Header
// ===============
//
// .. code-block:: cpp

#ifndef ch_protocol_h
#define ch_protocol_h

#include "../include/chirp_obj.h"
#include "connection.h"
#include "sglib.h"

// .. c:type:: ch_receipt_t
//
//    Receipt set implemented as rbtree.
//
//    .. c:member:: unsigned char receipt[16]
//
//    the receipt generated using TODO
//
// .. code-block:: cpp

typedef struct ch_receipt_s {
  unsigned char        receipt[16];
  char                 color_field;
  struct ch_receipt_s* left;
  struct ch_receipt_s* right;
} ch_receipt_t;

// .. c:type:: ch_protocol_t
//
//    Protocol object.
//
//    .. c:member:: struct sockaddr_in addrv4
//
//       BIND_V4 addr converted to a sockaddr_in
//
//    .. c:member:: struct sockaddr_in addrv4
//
//       BIND_V6 addr converted to a sockaddr_in6
//
//    .. c:member:: uv_tcp_t serverv4
//
//       reference to the libuv tcp server
//
//    .. c:member:: uv_tcp_t serverv6
//
//       reference to the libuv tcp server
//
//       TODO Complete
//
// .. code-block:: cpp

typedef struct ch_protocol_s {
    struct sockaddr_in  addrv4;
    struct sockaddr_in6 addrv6;
    uv_tcp_t            serverv4;
    uv_tcp_t            serverv6;
    ch_connection_t*    connections;
    ch_connection_t*    old_connections;
    ch_receipt_t*       receipts;
    ch_receipt_t*       late_receipts;
    ch_chirp_t*         chirp;
} ch_protocol_t;


#define CH_RECEIPT_CMP(x,y) \
    memcmp(x->receipt, y->receipt, 16)

SGLIB_DEFINE_RBTREE_PROTOTYPES(
    ch_receipt_t,
    left,
    right,
    color_field,
    CH_RECEIPT_CMP
)

// .. c:function::
static
ch_inline
void
ch_pr_init(ch_chirp_t* chirp, ch_protocol_t* protocol)
//
//    Initialize the protocol struct.
//
//    :param ch_chirp_t* chirp: Chirp instance
//    :param ch_protocol_t* protocol: Protocol to initialize
//
// .. code-block:: cpp
//
{
    memset(protocol, 0, sizeof(ch_protocol_t));
    protocol->chirp = chirp;
}

// .. c:function::
ch_error_t
ch_pr_start(ch_protocol_t* protocol);
//
//    Start the protocol
//
//    TODO params
//

// .. c:function::
ch_error_t
ch_pr_stop(ch_protocol_t* protocol);
//
//    Stop the protocol
//
//    TODO params
//
// .. code-block:: cpp

#endif //ch_protocol_h
