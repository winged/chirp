// =========
// Constants
// =========
//
// Defines constants used throughout the project.
//
// .. code-block:: cpp

#ifndef ch_libchirp_const_h
#define ch_libchirp_const_h

// Declarations
// ============

// .. c:type:: ch_ip_protocol_t
//
//    IP protocol definition. This is either IPV4 or IPV6.
//
//    .. c:member:: CH_IPV4
//
//       Defines the usage of IP protocol version 4.
//
//    .. c:member:: CH_IPV6
//
//       Defines the usage of IP protocol version 6.
//
// .. code-block:: cpp
//
typedef enum {
    CH_IPV4     = 0,
    CH_IPV6     = 1
} ch_ip_protocol_t;

#endif //ch_libchirp_const_h
