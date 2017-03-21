// =====
// Chirp
// =====
//
// .. todo:: Document purpose
//
// .. code-block:: cpp
//
#ifndef ch_libchirp_chirp_h
#define ch_libchirp_chirp_h

// Project includes
// ================
//
// .. code-block:: cpp
//
#include "common.h"
#include "callbacks.h"
#include "wrappers.h"
#include "message.h"

// Declarations
// ============

// .. c:type:: ch_config_t
//
//    Chirp configuration.
//
//    .. c:member:: float REUSE_TIME
//
//       Time until a connection gets garbage collected. After this the
//       connection will be reused.
//
//    .. c:member:: float TIMEOUT
//
//       General IO related timeout.
//
//    .. c:member:: uint16_t PORT
//
//       Port for listening to connections.
//
//    .. c:member:: uint8_t BACKLOG
//
//       TCP-listen socket backlog.
//
//    .. c:member:: uint8_t RETRIES
//
//       Count of retries till error is reported, by default 1.
//
//    .. c:member:: uint8_t MAX_HANDLERS
//
//       Count of handlers used. Allowed values are values between 1 and 32.
//       The default value is 16. If FLOW_CONTROL is on, it must be >= 16.
//
//    .. c:member:: char ACKNOWLEDGE
//
//       Acknowledge messages. Is needed for retry and flow-control. Disabling
//       acknowledge can improve performance on long delay connections, but at
//       the risk of overloading the remote and the local machine. You have to
//       set RETRIES and FLOW_CONTROL to 0 or chirp won't accept the config.
//
//    .. c:member:: char FLOW_CONTROL
//
//       Flow control prevents overloading of a node in a chain for workers.
//       Default: 1.
//
//    .. c:member:: char CLOSE_ON_SIGINT
//
//       By default chirp closes on SIGINT (Ctrl-C).
//
//    .. c:member:: uint32_t BUFFER_SIZE
//
//       Size of the buffer used for a connection. Defaults to 0, which means
//       use the size requested by libuv. Should not be set below 1024.
//
//    .. c:member:: uint8_t[16] BIND_V6
//
//       Override IPv6 bind address.
//
//    .. c:member:: uint8_t[4] BIND_V4
//
//       Override IPv4 bind address.
//
//    .. c:member:: uint8_t[16] IDENTITY
//
//       Override the IDENTITY. By default all chars are 0, which means chirp
//       will generate a IDENTITY.
//
//    .. c:member:: char* CERT_CHAIN_PEM
//
//       Holds the verification certificate.
//
//    .. c:member:: char* DH_PARAMS_PEM
//
//       Holds the path to the file containing DH parameters.
//
//    .. c:member:: char DISABLE_ENCRYPTION
//
//       Disables encryption. Only use if you know what you are doing.
//       Connections to "127.0.0.1" and "::1" aren't encrypted anyways.
//
// .. code-block:: cpp
//
struct ch_config_s {
    float           REUSE_TIME;
    float           TIMEOUT;
    uint16_t        PORT;
    uint8_t         BACKLOG;
    uint8_t         RETRIES;
    uint8_t         MAX_HANDLERS;
    char            ACKNOWLEDGE;
    char            FLOW_CONTROL;
    char            CLOSE_ON_SIGINT;
    uint32_t        BUFFER_SIZE;
    uint8_t         BIND_V6[CH_IP_ADDR_SIZE];
    uint8_t         BIND_V4[CH_IP4_ADDR_SIZE];
    uint8_t         IDENTITY[CH_ID_SIZE]; // 16
    char*           CERT_CHAIN_PEM;
    char*           DH_PARAMS_PEM;
    char            DISABLE_ENCRYPTION;
};

// .. c:type:: ch_chirp_int_t
//    :noindex:
//
//    Opaque pointer to internals.
//
//    see: :c:type:`ch_chirp_int_t`
//
// .. code-block:: cpp
//
typedef struct ch_chirp_int_s ch_chirp_int_t;

// .. c:type:: ch_chirp_t
//
//    Chirp object. It has no public members and uses an opaque pointer to its
//    internal data structures.
//
// .. code-block:: cpp
//
struct ch_chirp_s {
    ch_chirp_int_t*    _;
    ch_log_cb_t        _log;
    int                _init;
    uv_async_t         _done;
    ch_done_cb_t       _done_cb;
    char               _color_field;
    struct ch_chirp_s* _left;
    struct ch_chirp_s* _right;
};

// .. c:type:: ch_identity_t
//
//    Struct containing the chirp identity.
//
//    .. c:member:: unsigned uint8_t[16] data
//
//       The chrip identity is uint8_t array of length 16.
//
// .. code-block:: cpp
//
typedef struct ch_identity_s {
    uint8_t data[CH_ID_SIZE];
} ch_identity_t;

// .. c:function::
extern
void
ch_chirp_config_init(ch_config_t* config);
//
//    Initialize chirp configuration with defaults.
//
//    :param ch_config_t* config: Pointer to a chirp configuration.

// .. c:function::
extern
ch_error_t
ch_chirp_close_ts(ch_chirp_t* chirp);
//
//    Clean up chirp object. This will remove all callbacks. Pending outs will
//    be ignored after calling free.
//
//    This function is thread-safe.
//
//    :param ch_chirp_t* chirp: Pointer to a chirp object.
//
//    :return: A chirp error. See: :c:type:`ch_error_t`.
//    :rtype: ch_error_t

// .. c:function::
extern
ch_identity_t
ch_chirp_get_identity(ch_chirp_t* chirp);
//
//    Get the identity of the given chirp instance.
//
//    :param ch_chirp_t* chirp: Pointer to a chirp object.
//
//    :return: a chirp identity. See: :c:type:`ch_identity_t`.
//    :rtype:  ch_identity_t

// .. c:function::
extern
uv_loop_t*
ch_chirp_get_loop(ch_chirp_t* chirp);
//
//    Get the loop of the given chirp object.
//
//    :param ch_chirp_t* chirp: Pointer to a chirp object.
//
//    :return: a pointer to a libuv event loop object.
//    :rtype:  uv_loop_t*

// .. c:function::
extern
ch_error_t
ch_chirp_init(
        ch_chirp_t* chirp,
        const ch_config_t* config,
        uv_loop_t* loop,
        ch_start_cb_t start,
        ch_done_cb_t done,
        ch_log_cb_t log_cb
);
//
//    Initialiaze a chirp object. Memory is provided by the caller. You must call
//    :c:func:`ch_chirp_close` to cleanup the chirp object.
//
//    You can free **chirp**, **config** and **loop** either after the **done**
//    callback has been called or if chirp is set to auto-stop, after the loop
//    has finished.
//
//    Please call
//
//    .. code-block: cpp
//
//       ch_loop_close(loop)
//
//    before freeing the loop. Of course if the loop will continue to run, feel
//    free not to close/free the loop.
//
//    :param ch_chirp_t* chirp: Out: Pointer to a chirp object.
//    :param ch_config_t* config: Pointer to a chirp configration.
//    :param uv_loop_t* loop: Reference to a libuv loop.
//    :param ch_start_cb_t done: Called when chirp is started, can be NULL.
//    :param ch_done_cb_t done: Called when chirp is finished, can be NULL.
//    :param ch_log_cb_t log_cb: Callback to the logging facility, can be NULL.
//
//    :return: A chirp error. See: :c:type:`ch_error_t`.
//    :rtype: ch_error_t
//

// Definitions
// ===========

// .. c:function::
static
ch_inline
void
ch_chirp_register_log_cb(ch_chirp_t* chirp, ch_log_cb_t log_cb)
//
//    Register a callback for sending log messages.
//
//    :param ch_chirp_t* chirp: Pointer to a chirp object.
//    :param ch_log_cb_t   log: Callback to be called when logging messages.
//
// .. code-block:: cpp
//
{
    chirp->_log = log_cb;
}

// .. c:function::
extern
ch_error_t
ch_chirp_run(
        const ch_config_t* config,
        ch_chirp_t** chirp,
        ch_start_cb_t start,
        ch_log_cb_t log
);
//
//    Initializes, runs and cleans everything. Everything being:
//
//      * chirp object
//      * uv-loop
//      * uv-sockets
//      * callbacks
//
//     The method blocks, but chirp paramenter will be set. Can be used to run
//     chirp in a user defined thread. Use :c:func:`ch_chirp_close_ts` to close
//     chirp in any other thread.
//
//    :param ch_config_t* config: Pointer to a chirp configuration.
//    :param ch_chirp_t** chirp: Out: Pointer to a chirp object pointer. Can be
//                               NULL.
//
//    :return: A chirp error. See: :c:type:`ch_error_t`.
//    :param ch_log_cb_t log: Callback to be called when logging messages.
//    :rtype: ch_error_t

// .. c:function::
extern
void
ch_chirp_send(ch_chirp_t* chirp, ch_message_t* msg, ch_send_cb_t send_cb);
//
//    Send a message. Messages can be sent in parallel to different nodes.
//    Messages to the same node will block if you don't wait for the callback.
//
//    If you don't want to allocate messages on sending, we recommend to use a
//    pool of messages.
//
//    :param ch_chirp_t* chirp: Pointer to a chirp object.
//    :param ch_message_t msg: The message to send. The memory of the message
//                             must stay valid until the callback is called.
//    :param ch_send_cb_t send_cb: The callback, that will be called after
//                                 sending.

// .. c:function::
extern
void
ch_chirp_set_auto_stop_loop(ch_chirp_t* chirp);
//
//    Tells chirp to stop the uv-loop when closing (by setting the
//    corresponding flag).
//
//    After this function is called, :c:func:`ch_chirp_close_ts` will also stop
//    the loop.
//
//    This function is thread-safe.
//
//    :param ch_chirp_t* chirp: Pointer to a chirp object.
//
// .. code-block:: cpp

#endif //ch_libchirp_chirp_h
