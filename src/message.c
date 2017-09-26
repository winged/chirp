// =======
// Message
// =======
//

// Project includes
// ================
//
// .. code-block:: cpp
//
#include "message.h"
#include "util.h"

// Definitions
// ===========

// Queue definition
// ----------------
//

qs_queue_bind_impl_cx_m(ch_msg, ch_message_t)

// Interface definitions
// ---------------------

// .. c:function::
CH_EXPORT
ch_error_t
ch_msg_get_address(
        const ch_message_t* message,
        ch_text_address_t* address
)
//    :noindex:
//
//    see: :c:func:`ch_msg_get_address`
//
// .. code-block:: cpp
//
{
    int af;
    switch(message->ip_protocol) {
        case CH_IPV4:
            af = AF_INET;
            break;
        case CH_IPV6:
            af = AF_INET6;
            break;
        default:
            return CH_PROTOCOL_ERROR;
    }
    if(uv_inet_ntop(
            af, message->address, address->data, sizeof(ch_text_address_t)
    )) {
        return CH_PROTOCOL_ERROR;
    }
    return CH_SUCCESS;
}

// .. c:function::
CH_EXPORT
ch_error_t
ch_msg_init(ch_chirp_t* chirp, ch_message_t* message)
//    :noindex:
//
//    see: :c:func:`ch_msg_init`
//
// .. code-block:: cpp
//
{
    memset(message, 0, sizeof(ch_message_t));
    ch_random_ints_as_bytes(
        message->identity,
        sizeof(message->identity)
    );
    message->chirp = chirp;
    return CH_SUCCESS;
}

// .. c:function::
CH_EXPORT
ch_error_t
ch_msg_set_address(
        ch_message_t* message,
        ch_ip_protocol_t ip_protocol,
        const char* address,
        int32_t port
)
//    :noindex:
//
//    see: :c:func:`ch_msg_set_address`
//
// .. code-block:: cpp
//
{
    int af;
    message->ip_protocol = ip_protocol;
    switch(ip_protocol) {
        case CH_IPV4:
            af = AF_INET;
            break;
        case CH_IPV6:
            af = AF_INET6;
            break;
        default:
            return CH_VALUE_ERROR;
    }
    if(uv_inet_pton(af, address, message->address)) {
        return CH_VALUE_ERROR;
    }
    message->port = port;
    return CH_SUCCESS;
}
