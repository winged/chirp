// =======
// Message
// =======
//
// .. todo:: Document purpose
//
// .. code-block:: cpp
//
#ifndef ch_libchirp_message_h
#define ch_libchirp_message_h

// Project includes
// ================
//
// .. code-block:: cpp
//
#include "common.h"
#include "callbacks.h"

// System includes
// ===============
//
// .. code-block:: cpp
//
#ifdef _WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
#else // _WIN32
#include <arpa/inet.h>
#endif // _WIN32

// Declarations
// ============

// .. c:macro:: CH_WIRE_MESSAGE
//
//    Defines a chirp-wire message.
//
//    .. c:member:: uint8_t[16] identity
//
//       The identity of the message.
//
//    .. c:member:: uint8_t[16] serial
//
//       The serial number of the message.
//
//    .. c:member:: uint8_t message_type
//
//       The type of the message.
//
//    .. c:member:: uint16_t header_len
//
//       Length of the message header.
//
//    .. c:member:: uint16_t actor_len
//
//       Length of the actor. Defines the encoding of the actor. The default
//       actor is encoded as :code:`actor_len = 0`.
//
//    .. c:member:: uint32_t data_len
//
//       Length of the data the message contains.
//
// .. code-block:: cpp
//
#define CH_WIRE_MESSAGE \
    uint8_t  identity[CH_ID_SIZE]; \
    uint8_t  serial[CH_ID_SIZE]; \
    uint8_t  message_type; \
    uint16_t header_len; \
    uint16_t actor_len; \
    uint32_t data_len \

// .. c:type:: ch_message_t
//
//    Represents a message.
//
//    .. c:member:: CH_WIRE_MESSAGE
//
//       Wire-specific details about the message, such as identity, serial, type
//       and data length. See :c:macro:`CH_WIRE_MESSAGE`.
//
//    .. c:member:: ch_buf* header
//
//       Header of the message defined as (char-) buffer.
//
//    .. c:member:: char* actor
//
//       The actor of the message. The actor is defined by the actor length,
//       :c:member:`actor_len` and its default encoding is
//       :code:`actor_len = 0`. An actor is an universal primitive of concurrent
//       computation.
//
//    .. c:member:: ch_buf* data
//
//       The data of the message as pointer to a buffer.
//
//    .. c:member:: uint8_t ip_protocol
//
//       The IP protocol which was / shall be used for this message. This may
//       either be IPv4 or IPv6. See :c:type:`ch_ip_protocol_t`.
//
//    .. c:member:: uint8_t[16] address
//
//       IPv4/6 address of the sender if the message was received. IPv4/6
//       address of the recipient if the message is going to be sent.
//
//    .. c:member:: int32_t port
//
//       The port that the will be used reading/writing a message over a
//       connection.
//
//    .. c:member:: int8_t free_header
//
//       Unused.
//       .. todo:: Unused.
//
//    .. c:member:: int8_t free_actor
//
//       Unused.
//       .. todo:: Unused.
//
//    .. c:member:: int8_t free_data
//
//       Unused.
//       .. todo:: Unused.
//
//    .. c:member:: ch_chirp_t* chirp
//
//       Pointer to chirp instance.
//
// .. code-block:: cpp
//
struct ch_message_s {
    // Network data, has to be sent in network order
    CH_WIRE_MESSAGE;
    // These fields follow the message in this order (see _len above)
    ch_buf*     header;
    char*       actor;
    ch_buf*     data;
    // Local    only data
    uint8_t     ip_protocol;
    uint8_t     address[CH_IP_ADDR_SIZE];  // 16
    int32_t     port;
    int8_t      free_header;
    int8_t      free_actor;
    int8_t      free_data;
    ch_chirp_t* chirp;
    ch_send_cb_t _send_cb;
    void* _conn;
};

// .. c:type:: ch_msg_message_t
//
//    Wire message (network endianness), see :c:macro:`CH_WIRE_MESSAGE`.
//
// .. code-block:: cpp
//
typedef struct ch_msg_message_s {
    CH_WIRE_MESSAGE;
} ch_msg_message_t;

// Protocol receiver /Pseudo code/
//
// .. code-block:: cpp
//
//    ch_message_t msg;
//    recv_wait(buffer=&msg, size=39)
//    msg.actor = malloc(msg.actor_len) *
//    if(msg.header_len) {
//        msg.header = malloc(msg.header_len) *
//        recv_exactly(buffer=msg.header, msg.header_len)
//    }
//    if(msg.actor_len) {
//        msg.actor = malloc(msg.actor_len) *
//        recv_exactly(buffer=msg.actor, msg.actor_len)
//    }
//    if(msg.data_len) {
//        msg.data  = malloc(msg.data_len) *
//        recv_exactly(buffer=msg.data, msg.data_len)
//    }
//
// * Please use MAX_HANDLERS preallocated buffers of size 32 for header
// * Please use MAX_HANDLERS preallocated buffers of size 256 for actor
// * Please use MAX_HANDLERS preallocated buffers of size 512 for data
//
// Either fields may exceed the limit, in which case you have to alloc and set
// the free_* field.
//
// The default actor is encoded as actor_len = 0

// .. c:function::
CH_EXPORT
ch_error_t
ch_msg_get_address(
    const ch_message_t* message,
    ch_text_address_t* address
);
//
//    Get the messages' address which is an IP-address. The port and
//    ip_protocol can be read directly from the message. Address must be of the
//    size INET(6)_ADDRSTRLEN.
//
//    :param ch_message_t* message: Pointer to the message
//    :param ch_text_address_t* address: Textual representation of IP-address
//
//    :return: A chirp error. see: :c:type:`ch_error_t`
//    :rtype:  ch_error_t

// .. c:function::
CH_EXPORT
ch_error_t
ch_msg_init(ch_chirp_t* chirp, ch_message_t* message);
//
//    Initialize a message. Memory provided by caller (for performance).
//
//    :param ch_chirp_t* chirp: Pointer to chirp
//    :param ch_message_t* message: Pointer to the message
//
//    :return: A chirp error. see: :c:type:`ch_error_t`
//    :rtype:  ch_error_t

// .. c:function::
extern
ch_error_t
ch_msg_set_address(
    ch_message_t* message,
    ch_ip_protocol_t ip_protocol,
    const char* address,
    int32_t port
);
//
//    Set the messages' address in terms of IP-address and port.
//
//    :param ch_message_t* message: Pointer to the message
//    :param ch_ip_protocol_t ip_protocol: IP-protocol of the address
//    :param char* address: Textual representation of IP
//    :param int32_t port: Port of the remote
//
//    :return: A chirp error. see: :c:type:`ch_error_t`
//    :rtype:  ch_error_t

// .. code-block:: cpp
//
#endif //ch_libchirp_message_h
