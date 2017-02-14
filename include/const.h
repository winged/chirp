// ======
// Consts
// ======
//
// .. code-block:: cpp

#ifndef ch_inc_const_h
#define ch_inc_const_h

// .. c:type:: ch_ip_protocol_t
//
//    Select IPV4 or IPV6
//  
//    .. c:member:: CH_IPV4
//
//       IPV4
//
//    .. c:member:: CH_IPV6
//
//       IPV6
//
// .. code-block:: cpp
//
typedef enum {
    CH_IPV4     = 0,
    CH_IPV6     = 1
} ch_ip_protocol_t;

#endif //ch_inc_const_h
