// ==================
// External functions
// ==================
//
// .. todo:: Document purpose
//
// .. code-block:: cpp

#ifndef ch_libchirp_wrappers_h
#define ch_libchirp_wrappers_h

// Project includes
// ================
//
// .. code-block:: cpp
//
#include "common.h"

// Declarations
// ============

// .. c:function::
CH_EXPORT
int
ch_loop_init(uv_loop_t* loop);
//
//    An alias for uv_loop_init. Please refer to the libuv documentation.
//
//    :param uv_loop_t* loop: Loop struct allocated by user.
//
//    :return: the status of the initialization.
//    :rtype:  int
//

// Definitions
// ===========

// .. c:function::
static
int
ch_loop_close(uv_loop_t* loop)
//
//    An alias for uv_loop_close. Please refer to the libuv documentation.
//
//    :param uv_loop_t* loop: Loop struct allocated by the user.
//
//    :return: the status of the closing action.
//    :rtype:  int
//
// .. code-block:: cpp
//
{
    int tmp_err;
    tmp_err = uv_loop_close(loop);
#if defined(CH_LOG_TO_STDERR) && !defined(NDEBUG)
    fprintf(
        stderr,
        "%s:%d Closing loop exitcode:%d. uv_loop_t:%p\n",
        __FILE__,
        __LINE__,
        tmp_err,
        (void*) loop
    );
#endif
    return tmp_err;
}

// .. c:function::
static
int
ch_run(uv_loop_t* loop)
//
//    A wrapper for uv_run.
//    Runs the loop once again, in case closing chirp's resources caused
//    additional requests/handlers.
//
//    Please refer to the libuv documentation.
//
//    :param uv_loop_t*  loop: Loop struct allocated by user.
//
//    :return: the status of the action.
//    :rtype:  int
//
// .. code-block:: cpp
//
{
    int tmp_err = uv_run(loop, UV_RUN_DEFAULT);
    if(tmp_err != 0) {
        /* On Windows uv_run returns non-zero, which I expect, since we
         * uv_stop() inside a handler that just closed all kinds of resources,
         * which may cause new requests/handlers.
         *
         * I didn't find any documents assuring that my solution is good,
         * though.
         */
        tmp_err = uv_run(loop, UV_RUN_ONCE);
        /* Now we clearly have a problem */
        if(tmp_err != 0) {
            fprintf(stderr, "FATAL: Cannot close all uv-handles.\n");
        }
    }
    return tmp_err;
}

// .. code-block:: cpp
//
#endif //ch_libchirp_wrappers_h
