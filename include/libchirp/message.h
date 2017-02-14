// =======
// Message
// =======
// 
// .. code-block:: cpp

#ifndef ch_libchirp_message_h
#define ch_libchirp_message_h

#include "common.h"

#ifdef _WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
#else // _WIN32
#include <arpa/inet.h>
#endif // _WIN32


// .. c:type:: ch_message_t
//
//    Represents a message.
//  
//    .. c:member:: uint8_t[16] address
//
//       IPv4/6 address of the sender if the message was received.  IPv4/6
//       address of the recipient if the message is going to be sent.
//
// .. code-block:: cpp

#define CH_WIRE_MESSAGE \
    uint8_t  identity[16]; \
    uint8_t  serial[16]; \
    uint8_t  message_type; \
    uint16_t header_len; \
    uint16_t actor_len; \
    uint32_t data_len \

typedef struct ch_message_s {
    // Network data, has to be sent in network order
    CH_WIRE_MESSAGE;
    // These fields follow the message in this order (see _len above)
    ch_buf*  header;
    char*    actor;
    ch_buf*  data;
    // Local only data
    uint8_t  ip_protocol;
    uint8_t  address[16];
    int32_t  port;
    int8_t   free_header;
    int8_t   free_actor;
    int8_t   free_data;
} ch_message_t;

// .. c:type:: ch_ms_message_t
//
//    Wire message (network endianness)
//
// .. code-block:: cpp

typedef struct ch_ms_message_s {
    CH_WIRE_MESSAGE;
} ch_ms_message_t;

// .. c:type:: ch_text_address_t
//
//    Type to be used with :c:func:`ch_msg_get_address`
//
// .. code-block:: cpp
//
typedef struct ch_text_address_s {
    char data[INET6_ADDRSTRLEN];
} ch_text_address_t;

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
extern
ch_error_t
ch_msg_get_address(
    const ch_message_t* message,
    ch_text_address_t* address
);
//
//    Get the messages' address: IP-address. The port and ip_protocol can be
//    read from the message directly. Address must be of the size
//    INET(6)_ADDRSTRLEN.
//
//    :param ch_message_t* message: Pointer to the message
//    :param ch_text_address_t* address: Out: Textual representation of IP
//
// .. c:function::
extern
ch_error_t
ch_msg_init(ch_message_t* message);
//
//    Intialiaze a message. Memory provided by caller (for performance).
//
//    :param ch_message_t* message: Pointer to the message
//
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
//    Set the messages' address: IP-address and port
//
//    :param ch_message_t* message: Pointer to the message
//    :param ch_ip_protocol_t ip_protocol: IP-protocol of the address
//    :param char* address: Textual representation of IP
//    :param int32_t port: Port of the remote
//
// .. code-block:: cpp

#endif //ch_libchirp_message_h
