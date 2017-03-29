// ==============
// Message header
// ==============
//
// .. todo:: Document purpose
//
// .. code-block:: cpp
//
#ifndef ch_msg_message_h
#define ch_msg_message_h

// Project includes
// ================
#include "libchirp/message.h"

// .. c:type:: ch_msg_flags_t
//
//    Represents message flags.
//
//    .. c:member:: CH_MSG_REQ_ACK
//
//       Message requires ack.
//
//    .. c:member:: CH_MSG_ACK
//
//       Message is an ack.
//
// .. code-block:: cpp
//
typedef enum {
    CH_MSG_REQ_ACK = 1 << 0,
    CH_MSG_ACK     = 1 << 1,
} ch_msg_flags_t;

#endif //ch_msg_message_h
