// =============
// Common header
// =============
//
// Defines global forward declarations and utility macros.
//
// .. code-block:: cpp
//
#ifndef ch_common_h
#define ch_common_h

// Project includes
// ================
//
// .. code-block:: cpp
//
#include "libchirp/common.h"
#include "libchirp-config.h"
#include "util.h"

// System includes
// ===============
//
// .. code-block:: cpp
//

#include <assert.h>
#include <limits.h>
#include <string.h>
#include <stdlib.h>
#include <stddef.h>
#include <stdio.h>

// Forward declarations
// =====================
//
// .. code-block:: cpp

struct ch_remote_s;
typedef struct ch_remote_s ch_remote_t;
struct ch_connection_s;
typedef struct ch_connection_s ch_connection_t;
struct ch_receipt_s;
typedef struct ch_receipt_s ch_receipt_t;

//
// .. _double-eval:
//
// Double evaluation
// =================
//
// In order to get proper file and line number we have to implement validation
// and assertion using macros. But macros are prone to double evaluation, if you
// pass function having side-effects. Passing a pure function or data is valid.
//
// So ONLY pass variables to the V and A macros!
//
// But what is double evaluation?
//     The macro will just copy the function call, passed to it. So it will
//     be evaluated inside the macro, which people will often forget, the
//     code will behave in unexpected ways. You might think that this is
//     obvious, but forgetting it so easy. Humans have no problem writing
//     something like this and be amazed that it doesn't work.
//
// .. code-block:: cpp
//
//    int i = generate_int();
//    /* A few lines in between, so it isn't that obvious */
//    V(chirp, generate_int() == 0, "The first int generated isn't zero");
//
// Which should look like this.
//
// .. code-block:: cpp
//
//    int i = generate_int();
//    V(chirp, i == 0, "The first int generated isn't zero");
//
// Definitions
// ===========

// .. c:macro:: EC
//
//    Reports an error.
//
//    The error macro E(chirp, message, ...) behaves like printf and allows to
//    log to a custom callback. Usually used to log into pythons logging
//    facility. On the callback it sets the argument error to true and it will
//    log to stderr if no callback is set.
//
//    :param chirp: Pointer to a chirp object.
//    :param message: The highlighted message to report.
//    :param clear:   The clear message to report.
//    :param ...: Variadic arguments for xprintf
//
// .. code-block:: cpp
//
#define EC(chirp, message, clear, ...) \
    ch_write_log(chirp, __FILE__, __LINE__, message, clear, 0, __VA_ARGS__);
#define E(chirp, message, ...) EC(chirp, message, "",  __VA_ARGS__)


// .. c:macro:: V
//
//    Validates the given condition and reports a message including arbitrary
//    given arguments when the condition is not met in debug-/development-mode.
//
//    Be aware of :ref:`double-eval`
//
//    The validate macro V(chirp, condition, message, ...) behaves like printf
//    and allows to print a message given an assertion. If that assertion is
//    not fullfilled, it will print the given message and return
//    :c:member:`ch_error_t.CH_VALUE_ERROR`, even in release mode.
//
//    :param chirp: Pointer to a chirp object.
//    :param condition: A boolean condition to check.
//    :param message: Message to print when the condition is not met.
//    :param ...: Variadic arguments for xprintf
//
// .. code-block:: cpp
//
#define V(chirp, condition, message, ...) \
    if(!(condition)) { \
        ch_write_log(chirp, __FILE__, __LINE__, message, "", 0, __VA_ARGS__); \
        assert(0); \
        return CH_VALUE_ERROR; \
    }

#ifndef NDEBUG

// .. c:macro:: LC
//
//    Logs the given message including arbitrary arguments to a custom callback
//    in debug-/development-mode.
//
//    The logging macro L(chirp, message, ...) behaves like printf and allows
//    to log to a custom callback. Usually used to log into pythons logging
//    facility.
//
//    :param chirp: Pointer to a chirp object.
//    :param message: The highlighted message to report.
//    :param clear:   The clear message to report.
//    :param ...: Variadic arguments for xprintf
//
// .. code-block:: cpp
//
#define LC(chirp, message, clear, ...) \
    ch_write_log(chirp, __FILE__, __LINE__, message, clear, 0, __VA_ARGS__);
#define L(chirp, message, ...) LC(chirp, message, "",  __VA_ARGS__)

// .. c:macro:: A
//
//    Validates the given condition and reports arbitrary arguments when the
//    condition is not met in debug-/development-mode.
//
//    Be aware of :ref:`double-eval`
//
//    The assert macro A(condition, ...) behaves like printf and allows to
//    print a arbitrary arguments when the given assertion fails.
//
//    :param condition: A boolean condition to check.
//    :param ...: Variadic arguments for xprintf
//
// .. code-block:: cpp
//
#define A(condition, ...) \
    if(!(condition)) { \
        fprintf(stderr, __VA_ARGS__); \
        assert(0); \
    }

// .. c:macro:: ch_chirp_check_m()
//
//    Validates that we have a valid chirp object and we are on the right
//    thread.
//
// .. code-block:: cpp

#define ch_chirp_check_m(chirp) \
    { \
        A(chirp->_init == CH_CHIRP_MAGIC, "Not a ch_chirp_t*"); \
        uv_thread_t __ch_chirp_check_self_ = uv_thread_self(); \
        A( \
            uv_thread_equal(&__ch_chirp_check_self_, &chirp->_thread) != 0, \
            "Call on the wrong thread" \
        ); \
    }

#else //NDEBUG

// .. c:macro:: L
//
//    See :c:macro:`L`. Does nothing in release-mode.
// .. code-block:: cpp
//
#   define L(chrip, message, ...) \
        (void)(chirp); (void)(message)
#   define LC(chrip, message, clear, ...) \
        (void)(chirp); (void)(message); (void)(clear)

//.. c:macro:: A
//
//    See :c:macro:`A`. Does nothing in release-mode.
//
//    Be aware of :ref:`double-eval`
//
// .. code-block:: cpp
//
#   define A(condition, ...) (void)(condition)
#   define ch_chirp_check_m(chirp) (void)(chirp)

#endif

#ifndef A
#   error Assert macro not defined
#endif
#ifndef ch_chirp_check_m
#   error ch_chirp_check_m macro not defined
#endif
#ifndef L
#   error Log macro L not defined
#endif
#ifndef LC
#   error Log macro LC not defined
#endif
#ifndef V
#   error Validate macro not defined
#endif


#define CH_CHIRP_MAGIC 42429

// Generic functions
// =================
//
// .. code-block:: text
//
#define MINMAX_FUNCS(type) \
    static \
    type \
    ch_max_##type(type a, type b) { \
        return (a > b) ? a : b; \
    } \
\
    static \
    type \
    ch_min_##type(type a, type b) { \
        return (a < b) ? a : b; \
    }

#endif //ch_common_h
