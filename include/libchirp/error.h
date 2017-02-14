// ======
// Errors
// ======
//
// .. code-block:: cpp

#ifndef ch_libchirp_error_h
#define ch_libchirp_error_h

// .. c:type:: ch_error_t
//
//    Represents a chirp error.
//
//    .. c:member:: CH_SUCCESS
//
//       No error
//
//    .. c:member:: CH_VALUE_ERROR
//
//       Supplied value is not allowed
//
//    .. c:member:: CH_UV_ERROR
//
//       General libuv error
//
//    .. c:member:: CH_PROTOCOL_ERROR
//
//       Happens when bad values are received or remote dies unexpectedly
//
//    .. c:member:: CH_EADDRINUSE
//
//       The chirp port was already in use
//
//    .. c:member:: CH_FATAL
//
//       # TODO list cases
//
//       * We do not have a entropy source
//
//    .. c:member:: CH_TLS_ERROR
//
//       General TLS error
//
//    .. c:member:: CH_UNINIT
//
//       Chirp or another object is not initialized
//
//    .. c:member:: CH_IN_PRORESS
//
//       Action is already in progress
//
// .. code-block:: cpp
//
typedef enum {
    CH_SUCCESS        = 0,
    CH_VALUE_ERROR    = 1,
    CH_UV_ERROR       = 2,
    CH_PROTOCOL_ERROR = 3,
    CH_EADDRINUSE     = 4,
    CH_FATAL          = 5,
    CH_TLS_ERROR      = 6,
    CH_UNINIT         = 7,
    CH_IN_PRORESS     = 8,
    CH_TIMEOUT        = 9,
} ch_error_t;

#endif //ch_libchirp_error_h
