// =================
// Serializer header
// =================
//
// * Convert the CH_WIRE_MESSAGE to a buffer in network order
// * Convert a buffer in network order to CH_WIRE_MESSAGE
//
// CH_WIRE_MESSAGE refers to a part of ch_message_t
//
// * Convert ch_sr_handshake_t to a buffer in network order
// * Convert a buffer in network order to ch_sr_handshake_t
//
// .. code-block:: cpp

#ifndef ch_serializer_h
#define ch_serializer_h

// Project includes
// ================
//
// .. code-block:: cpp

#include "common.h"
#include "libchirp/message.h"

// Declarations
// ============

// .. c:type:: ch_sr_handshake_t
//
//    Handshake data structure.
//
//    .. c:member:: uint16_t port
//
//       Public port which is passed to a connection on a successful handshake.
//
//    .. c:member:: uint16_t max_timeout
//
//       The maximum number of seconds to wait when connecting until a timeout
//       gets triggered. This is passed to a connection upon a successful
//       handshake. The value gets computed by the configured number of retries
//       incremented by two times the configured timeout value.
//
//    .. c:member:: uint8_t[16] identity
//
//       The identity of the remote target which is passed to a connection upon
//       a successful handshake. It is used by the connection for getting the
//       remote address.
//
// .. code-block:: cpp

typedef struct ch_sr_handshake_s {
    uint16_t port;
    uint16_t max_timeout;
    uint8_t  identity[CH_ID_SIZE];
} ch_sr_handshake_t;

// Definitions
// ===========
//
// .. code-block:: cpp

#define CH_SR_WIRE_MESSAGE_SIZE 27

#ifndef NDEBUG
#   begindef CH_SR_WIRE_MESSAGE_CHECK
        pos += 4;
        A(
            pos == CH_SR_WIRE_MESSAGE_SIZE,
            "Bad message serialization size"
        );
#   enddef
#else
#   define CH_SR_WIRE_MESSAGE_CHECK
#endif


#begindef CH_SR_WIRE_MESSAGE_LAYOUT
    size_t pos = 0;
    uint8_t* identity    = (void*) &buf[pos];
    pos += CH_ID_SIZE;

    uint32_t* serial      = (void*) &buf[pos];
    pos += 4;

    uint8_t* type        = (void*) &buf[pos];
    pos += 1;

    uint16_t* header_len = (void*) &buf[pos];
    pos += 2;

    uint32_t* data_len   = (void*) &buf[pos];
    CH_SR_WIRE_MESSAGE_CHECK
#enddef


// .. c:function::
static
ch_inline
int
ch_sr_buf_to_msg(ch_buf* buf, ch_message_t* msg)
//
//    Convert a buffer containing the packed data of CH_WIRE_MESSAGE in network
//    order to an ch_message_t.
//
//    :param ch_buf* buf: Pointer to the packed buffer of at least
//                        CH_SR_WIRE_MESSAGE_SIZE
//    :param ch_message_t* msg: Pointer to the message
//
// .. code-block:: cpp
//
{
    CH_SR_WIRE_MESSAGE_LAYOUT;

    memcpy(msg->identity, identity, CH_ID_SIZE);

    msg->type       = *type;
    msg->header_len = ntohs(*header_len);
    msg->data_len   = ntohl(*data_len);
    msg->serial     = ntohl(*serial);
    return CH_SR_WIRE_MESSAGE_SIZE;
}

// .. c:function::
static
ch_inline
int
ch_sr_msg_to_buf(ch_message_t* msg, ch_buf* buf)
//
//    Convert a ch_message_t to a buffer containing the packed data of
//    CH_WIRE_MESSAGE in network order.
//
//    :param ch_message_t* msg: Pointer to the message
//    :param ch_buf* buf: Pointer to the packed buffer of at least
//                        CH_SR_WIRE_MESSAGE_SIZE
//
// .. code-block:: cpp
//
{
    CH_SR_WIRE_MESSAGE_LAYOUT;

    memcpy(identity, msg->identity, CH_ID_SIZE);

    *type       = msg->type;
    *header_len = htons(msg->header_len);
    *data_len   = htonl(msg->data_len);
    *serial     = htonl(msg->serial);
    return CH_SR_WIRE_MESSAGE_SIZE;
}

#define CH_SR_HANDSHAKE_SIZE 20

#ifndef NDEBUG
#   begindef CH_SR_HANDSHAKE_CHECK
        pos += CH_ID_SIZE;
        A(
            pos == CH_SR_HANDSHAKE_SIZE,
            "Bad handshake serialization size"
        );
#   enddef
#else
#   define CH_SR_HANDSHAKE_CHECK
#endif


#begindef CH_SR_HANDSHAKE_LAYOUT
    size_t pos = 0;
    uint16_t* port        = (void*) &buf[pos];
    pos += 2;

    uint16_t* max_timeout = (void*) &buf[pos];
    pos += 2;

    uint8_t* identity     = (void*) &buf[pos];
    CH_SR_HANDSHAKE_CHECK
#enddef


// .. c:function::
static
ch_inline
int
ch_sr_buf_to_hs(ch_buf* buf, ch_sr_handshake_t* hs)
//
//    Convert a buffer containing the packed data of a handshake in network
//    order to an ch_sr_handshake_t.
//
//    :param ch_buf* buf: Pointer to the packed buffer of at least
//                        CH_SR_HANDSHAKE_SIZE
//    :param ch_sr_handshake_t: Pointer to the handshake struct
//
// .. code-block:: cpp
//
{
    CH_SR_HANDSHAKE_LAYOUT;

    hs->port = ntohs(*port);
    hs->max_timeout = ntohs(*max_timeout);
    memcpy(hs->identity, identity, CH_ID_SIZE);

    return CH_SR_HANDSHAKE_SIZE;
}

// .. c:function::
static
ch_inline
int
ch_sr_hs_to_buf(ch_sr_handshake_t* hs, ch_buf* buf)
//
//    Convert a ch_sr_handshake_t to  a buffer containing the packed data of
//    the handshake in network order.
//
//    :param ch_sr_handshake_t: Pointer to the handshake struct
//    :param ch_buf* buf: Pointer to the packed buffer of at least
//                        CH_SR_HANDSHAKE_SIZE
//
// .. code-block:: cpp
//
{
    CH_SR_HANDSHAKE_LAYOUT;

    *port = htons(hs->port);
    *max_timeout = ntohs(hs->max_timeout);
    memcpy(identity, hs->identity, CH_ID_SIZE);

    return CH_SR_HANDSHAKE_SIZE;
}

#endif //ch_serializer_h
