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
    int ip_protocol = message->ip_protocol;
    if(!(ip_protocol == AF_INET || ip_protocol == AF_INET6)) {
        return CH_VALUE_ERROR;
    }
    if(uv_inet_ntop(
            ip_protocol,
            message->address,
            address->data,
            sizeof(address->data)
    )) {
        return CH_PROTOCOL_ERROR;
    }
    return CH_SUCCESS;
}

// .. c:function::
CH_EXPORT
int
ch_msg_has_recv_handler(ch_message_t* message)
//    :noindex:
//
//    see: :c:func:`ch_msg_has_recv_handler`
//
// .. code-block:: cpp
//
{
    return message->_flags & CH_MSG_IS_HANDLER;
}

// .. c:function::
CH_EXPORT
ch_error_t
ch_msg_init(ch_message_t* message)
//    :noindex:
//
//    see: :c:func:`ch_msg_init`
//
// .. code-block:: cpp
//
{
    memset(message, 0, sizeof(*message));
    ch_random_ints_as_bytes(message->identity, sizeof(message->identity));
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
    message->ip_protocol = ip_protocol;
    if(!(ip_protocol == AF_INET || ip_protocol == AF_INET6)) {
        return CH_VALUE_ERROR;
    }
    if(uv_inet_pton(ip_protocol, address, message->address)) {
        return CH_VALUE_ERROR;
    }
    message->port = port;
    return CH_SUCCESS;
}
