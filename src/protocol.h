// ===============
// Protocol Header
// ===============
//
// .. todo:: Document purpose
//
// .. code-block:: cpp

#ifndef ch_protocol_h
#define ch_protocol_h

// Project includes
// ================
//
// .. code-block:: cpp
//
#include "libchirp/chirp.h"
#include "connection.h"
#include "sglib.h"

// Sglib Prototypes
// ================

// .. c:macro:: CH_RECEIPT_CMP
//
//    Compares two receipts memorywise as byte strings, assuming that both byte
//    strings are 16 bytes long.
//
//    :param x: First red-black tree node to get receipt (byte string) from.
//    :param y: Second red-black tree node to get receipt (byte string) from.
//
// .. code-block:: cpp
//
#define CH_RECEIPT_CMP(x,y) \
    memcmp(x->receipt, y->receipt, 16)

// .. code-block:: cpp
//
SGLIB_DEFINE_RBTREE_PROTOTYPES( // NOCOV
    ch_receipt_t,
    left,
    right,
    color_field,
    CH_RECEIPT_CMP
)

// Declarations
// ============

// .. c:type:: ch_receipt_t
//
//    Receipt set implemented as red-black tree.
//
//    .. c:member:: unsigned char receipt[16]
//
//       Identity of the receipt, represented as 16 byte long byte string.
//
//    .. c:member:: char color_field
//
//       The color of the current (protocol-) node. This may either be red or
//       black, as receipts are built as a red-black tree.
//
//    .. c:member:: ch_receipt_s* left
//
//       (Struct-) Pointer to the left child of the current receipt (node)
//       in the red-black tree.
//
//    .. c:member:: ch_receipt_s* right
//
//       (Struct-) Pointer to the right child of the current receipt (node)
//       in the red-black tree.
//
// .. code-block:: cpp
//
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
//       BIND_V4 address converted to a sockaddr_in.
//
//    .. c:member:: struct sockaddr_in addrv6
//
//       BIND_V6 address converted to a sockaddr_in6.
//
//    .. c:member:: uv_tcp_t serverv4
//
//       Reference to the libuv tcp server handle, IPv4.
//
//    .. c:member:: uv_tcp_t serverv6
//
//       Reference to the libuv tcp server handle, IPv6.
//
//    .. c:member:: ch_connection_t* connections
//
//       Pointer to connections that are used for this protocol.
//
//    .. c:member:: ch_connection_t* old_connections
//
//       Pointer to old connections. This is mainly used when there is a
//       network race condition. The then current connections will be replaced
//       and saved as old connections for garbage collection.
//
//    .. c:member:: ch_receipt_t* receipts
//
//       Pointer to a set of receipts.
//
//       A receipt gets added whenever the reader receives a message, that
//       requires an ACK. In that case, if a connections has a receipt set,
//       that receipt is removed from the set of (currently valid) receipts and
//       the serial number (identifier) of the the current message is added as
//       a new receipt.
//
//       A receipt is attached to a connection, whereas a protocol may have
//       multiple connections.
//
//    .. c:member:: ch_receipt_t* late_receipts
//
//       Pointer to a set of late receipts.
//
//       A late receipt gets added when a read on a certain connection was
//       cancelled. This might happen on events like timeouts, incomplete
//       reads, resets of the connection, broken pipes, extra data or an
//       exception when unpacking data.
//       A late receipt acts as a pending task (a todo-item): Remove the
//       receipt with the same (message-) identifier after N seconds, if this
//       specific receipt is still (or again) in the queue of receipts.
//
//    .. c:member:: ch_chirp_t* chirp
//
//       Pointer to the chirp object. See: :c:type:`ch_chirp_t`.
//
// .. code-block:: cpp
//
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

// .. c:function::
ch_error_t
ch_pr_start(ch_protocol_t* protocol);
//
//    Start the given protocol.
//
//    :param ch_protocol_t* protocol: Protocol which shall be started..
//
//    :return: A chirp error. see: :c:type:`ch_error_t`
//    :rtype:  ch_error_t

// .. c:function::
ch_error_t
ch_pr_stop(ch_protocol_t* protocol);
//
//    Stop the given protocol.
//
//    :param ch_protocol_t* protocol: Protocol which shall be stopped.
//
//    :return: A chirp error. see: :c:type:`ch_error_t`
//    :rtype:  ch_error_t

// Definitions
// ===========

// .. c:function::
static
ch_inline
void
ch_pr_init(ch_chirp_t* chirp, ch_protocol_t* protocol)
//
//    Initialize the protocol structure.
//
//    :param ch_chirp_t* chirp: Chirp instance.
//    :param ch_protocol_t* protocol: Protocol to initialize.
//
// .. code-block:: cpp
//
{
    memset(protocol, 0, sizeof(ch_protocol_t));
    protocol->chirp = chirp;
}

// .. code-block:: cpp
//
#endif //ch_protocol_h
