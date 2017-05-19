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

// .. c:type:: ch_msg_type_t
//
//    Represents message type flags.
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
} ch_msg_type_t;

// .. c:type:: ch_msg_flags_t
//
//    Represents message flags.
//
//    .. c:member:: CH_MSG_FREE_HEADER
//
//       Header data has to be freed before releasing the buffer
//
//    .. c:member:: CH_MSG_FREE_ACTOR
//
//       Header data has to be freed before releasing the buffer
//
//    .. c:member:: CH_MSG_FREE_DATA
//
//       Data has to be freed before releasing the buffer
//
//    .. c:member:: CH_MSG_QUEUED
//
//       The message is being queue
//
//    .. c:member:: CH_MSG_USED
//
//       The message is used by chirp
//
// .. code-block:: cpp
//
typedef enum {
    CH_MSG_FREE_HEADER = 1 << 0,
    CH_MSG_FREE_ACTOR  = 1 << 1,
    CH_MSG_FREE_DATA   = 1 << 2,
    CH_MSG_QUEUED      = 1 << 3,
    CH_MSG_USED        = 1 << 4,
    CH_MSG_USER        = 1 << 5,
} ch_msg_flags_t;

#endif //ch_msg_message_h
