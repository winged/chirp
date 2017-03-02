// =============
// Common header
// =============
//
// I know common headers aren't good style but Windows is forcing me.
// .. todo:: Document purpose
//
// .. code-block:: cpp
//
#ifndef ch_libchirp_common_h
#define ch_libchirp_common_h

// Project includes
// ================
//
// .. code-block:: cpp
//
#include "error.h"
#include "const.h"

// System includes
// ===============
//
// .. code-block:: cpp
//
#include <uv.h>

#include <assert.h>
#include <limits.h>
#include <string.h>
#include <stdlib.h>
#include <stddef.h>
#include <stdio.h>

// Logging and assert macros
// =========================
//
// .. code-block:: cpp
//
#define CH_EMPTY

// .. c:macro:: E
//
//    Reports an error.
//
//    The error macro E(chirp, message, ...) behaves like printf and allows to
//    log to a custom callback. Usually used to log into pythons logging
//    facility. On the callback it sets the argument error to true and it will
//    log to stderr if no callback is set.
//
//    :param chirp: Pointer to a chirp object.
//    :param message: The message to report.
//    :param ...: Variadic arguments (arbitrary arguments of arbitrary
//                quantity).
//
// .. code-block:: cpp
//
#define E(chirp, message, ...) do { \
    if(chirp->_log != NULL) { \
        char buf[1024]; \
        snprintf( \
            buf, \
            1024, \
            "%s:%d " message, \
            __FILE__, \
            __LINE__, \
           __VA_ARGS__ \
        ); \
        chirp->_log(buf, 1); \
    } else { \
        fprintf( \
            stderr, \
            "%s:%d Error: " message "\n", \
            __FILE__, \
            __LINE__, \
            __VA_ARGS__ \
        ); \
    } \
} while(0)

#ifndef NDEBUG

// .. c:macro:: V
//
//    Validates the given condition and reports a message including arbitrary
//    given arguments when the condition is not met in debug-/development-mode.
//
//    The validate macro V(chirp, condition, message, ...) behaves like printf
//    and allows to print a message given an assertion. If that assertion is
//    not fullfilled, it will print the given message and return
//    :c:member:`ch_error_t.CH_VALUE_ERROR`, even in release mode.
//
//    :param chirp: Pointer to a chirp object.
//    :param condition: A boolean condition to check.
//    :param message: Message to print when the condition is not met.
//    :param ...: Variadic arguments (arbitrary arguments of arbitrary
//                quantity).
//
// .. code-block:: cpp
//
#   define V(chirp, condition, message, ...) do { \
        if(!(condition)) { \
            if(chirp->_log != NULL) { \
                char buf[1024]; \
                snprintf( \
                    buf, \
                    1024, \
                    "%s:%d " message, \
                    __FILE__, \
                    __LINE__, \
                    __VA_ARGS__ \
                ); \
                chirp->_log(buf, 1); \
            } else { \
                fprintf( \
                    stderr, \
                    "%s:%d " message "\n", \
                    __FILE__, \
                    __LINE__, \
                    __VA_ARGS__ \
                ); \
                assert(condition); \
                /* Since we check the condition twice, check for bad asserts*/ \
                fprintf(stderr, "Bad assert: condition not stable\n"); \
                assert(0); \
            } \
            return CH_VALUE_ERROR; \
        } \
    } while(0)

// .. c:macro:: VE
//
//    Validates the given condition and reports a message when the condition is
//    not met in debug-/development-mode.
//
//    The validate macro VE(chirp, condition, message) behaves like printf
//    and allows to print a message given an assertion. If that assertion is
//    not fullfilled, it will print the given message and return
//    :c:member:`ch_error_t.CH_VALUE_ERROR`, even in release mode.
//
//    :param chirp: Pointer to a chirp object.
//    :param condition: A boolean condition to check.
//    :param message: Message to print when the condition is not met.
//
// .. code-block:: cpp
//
#   define VE(chirp, condition, message) do { \
        if(!(condition)) { \
            if(chirp->_log != NULL) { \
                char buf[1024]; \
                snprintf( \
                    buf, \
                    1024, \
                    "%s:%d " message, \
                    __FILE__, \
                    __LINE__ \
                ); \
                chirp->_log(buf, 1); \
            } else { \
                fprintf( \
                    stderr, \
                    "%s:%d " message "\n", \
                    __FILE__, \
                    __LINE__ \
                ); \
                assert(condition); \
                /* Since we check the condition twice, check for bad asserts*/ \
                fprintf(stderr, "Bad assert: condition not stable\n"); \
                assert(0); \
            } \
            return CH_VALUE_ERROR; \
        } \
    } while(0)

// .. c:macro:: L
//
//    Logs the given message including arbitrary arguments to a custom callback
//    in debug-/development-mode.
//
//    The logging macro L(chirp, message, ...) behaves like printf and allows
//    to log to a custom callback. Usually used to log into pythons logging
//    facility.
//
//    :param chirp: Pointer to a chirp object.
//    :param message: Message to print when the condition is not met.
//    :param ...: Variadic arguments (arbitrary arguments of arbitrary
//                quantity).
//
// .. code-block:: cpp
//
#   define L(chirp, message, ...) do { \
        if(chirp->_log != NULL) { \
            char buf[1024]; \
            snprintf( \
                buf, \
                1024, \
                "%s:%d " message, \
                __FILE__, \
                __LINE__, \
                __VA_ARGS__ \
            ); \
            chirp->_log(buf, 0); \
        } else { \
            fprintf( \
                stderr, \
                "%s:%d " message "\n", \
                __FILE__, \
                __LINE__, \
                __VA_ARGS__ \
            ); \
        } \
    } while(0)

// .. c:macro:: A
//
//    Validates the given condition and reports arbitrary arguments when the
//    condition is not met in debug-/development-mode.
//
//    The assert macro A(condition, ...) behaves like printf and allows to
//    print a arbitrary arguments when the given assertion fails.
//
//    :param chirp: Pointer to a chirp object.
//    :param ...: Variadic arguments (arbitrary arguments of arbitrary
//                quantity).
//
// .. code-block:: cpp
//
#   define A(condition, ...) do { \
        if(!(condition)) { \
            fprintf(stderr, __VA_ARGS__); \
            fprintf(stderr, "\n"); \
            assert(condition); \
            /* Since we check the condition twice, check for bad asserts*/ \
            fprintf(stderr, "Bad assert: condition not stable\n"); \
            assert(0); \
        } \
    } while(0)
#else //NDEBUG

// .. c:macro:: V
//
//    Validates the given condition and reports a message including arbitrary
//    given arguments when the condition is not met in release-mode.
//
//    The validate macro V(chirp, condition, message, ...) behaves like printf
//    and allows to print a message given an assertion. If that assertion is
//    not fullfilled, it will print the given message and return
//    :c:member:`ch_error_t.CH_VALUE_ERROR`, even in release mode.
//
//    :param chirp: Pointer to a chirp object.
//    :param condition: A boolean condition to check.
//    :param message: Message to print when the condition is not met.
//    :param ...: Variadic arguments (arbitrary arguments of arbitrary
//                quantity).
//
// .. code-block:: cpp
//
#   define V(chirp, condition, message, ...) do { \
        if(!(condition)) { \
            if(chirp->_log != NULL) { \
                char buf[1024]; \
                snprintf( \
                    buf, \
                    1024, \
                    "%s:%d " message, \
                    __FILE__, \
                    __LINE__, \
                    __VA_ARGS__ \
                ); \
                chirp->_log(buf, 1); \
            } else { \
                fprintf( \
                    stderr, \
                    "%s:%d " message "\n", \
                    __FILE__, \
                    __LINE__, \
                    __VA_ARGS__ \
                ); \
            } \
            return CH_VALUE_ERROR; \
        } \
    } while(0)

// .. c:macro:: VE
//
//    Validates the given condition and reports a message when the condition is
//    not met in release-mode.
//
//    The validate macro VE(chirp, condition, message) behaves like printf
//    and allows to print a message given an assertion. If that assertion is
//    not fullfilled, it will print the given message and return
//    :c:member:`ch_error_t.CH_VALUE_ERROR`, even in release mode.
//
//    :param chirp: Pointer to a chirp object.
//    :param condition: A boolean condition to check.
//    :param message: Message to print when the condition is not met.
//
// .. code-block:: cpp
//
#   define VE(chirp, condition, message) do { \
        if(!(condition)) { \
            if(chirp->_log != NULL) { \
                char buf[1024]; \
                snprintf( \
                    buf, \
                    1024, \
                    "%s:%d " message, \
                    __FILE__, \
                    __LINE__ \
                ); \
                chirp->_log(buf, 1); \
            } else { \
                fprintf( \
                    stderr, \
                    "%s:%d " message "\n", \
                    __FILE__, \
                    __LINE__ \
                ); \
            } \
            return CH_VALUE_ERROR; \
        } \
    } while(0)

// .. c:macro:: L
//
//    See :c:macro:`L`. Does nothing in release-mode.
// .. code-block:: cpp
//
#   define L(chrip, message, ...) (void)(chirp); (void)(message)

//.. c:macro:: A
//
//    See :c:macro:`A`. Does nothing in release-mode.
//
// .. code-block:: cpp
//
#   define A(condition, ...) (void)(condition)

#endif


#ifndef A
#   error Assert macro not defined
#endif
#ifndef L
#   error Log macro not defined
#endif
#ifndef V
#   error Validate macro not defined
#endif

#define CH_CHIRP_MAGIC 42429

#define CH_GET_CHIRP(handle) \
ch_chirp_t* chirp = (ch_chirp_t*) handle->data; \
A(chirp->_init == CH_CHIRP_MAGIC, "Not a ch_chirp_t*")
/* Fail fast, best I know :( */

// Windows compatiblity
// ====================
//
// .. code-block:: cpp

#ifdef _WIN32
#   if defined(_MSC_VER) && _MSC_VER < 1600
#       include <stdint-msvc2008.h>
#       define ch_inline __inline
#   else // _MSC_VER
#       include <stdint.h>
#       define ch_inline inline
#   endif // _MSC_VER

#   if defined(_MSC_VER) && _MSC_VER < 1900

#       define snprintf c99_snprintf
#       define vsnprintf c99_vsnprintf

        __inline int c99_vsnprintf(
                char *outBuf,
                size_t size,
                const char *format,
                va_list ap
        )
        {
            int count = -1;

            if (size != 0)
                count = _vsnprintf_s(outBuf, size, _TRUNCATE, format, ap);
            if (count == -1)
                count = _vscprintf(format, ap);

            return count;
        }

        __inline int c99_snprintf(
                char *outBuf,
                size_t size,
                const char *format,
                ...
        )
        {
            int count;
            va_list ap;

            va_start(ap, format);
            count = c99_vsnprintf(outBuf, size, format, ap);
            va_end(ap);

            return count;
        }

#   endif // MSVC
#else // _WIN32
#   define ch_inline inline
#endif // _WIN32

// Typedefs
// ========
//
// .. code-block:: cpp

typedef char ch_buf; // Used to show it is not a c-string but a buffer

// Generic functions
// =================
//
// .. code-block:: cpp

#define MINMAX_FUNCS(type) \
static \
ch_inline \
type \
max_##type(type a, type b) { \
    return (a > b) ? a : b; \
} \
static \
ch_inline \
type \
min_##type(type a, type b) { \
    return (a < b) ? a : b; \
}


#endif //ch_libchirp_common_h
