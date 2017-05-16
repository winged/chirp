// =============
// Common header
// =============
//
// I know common headers aren't good style but Windows is forcing me.
// .. todo:: Document purpose
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

struct ch_connection_s;
typedef struct ch_connection_s ch_connection_t;
struct ch_receipt_s;
typedef struct ch_receipt_s ch_receipt_t;

// Logging and assert macros
// =========================
//
// .. _double-eval:
//
// Double evaluation
// ------------------
//
// In order to get proper file and line number we have to implement validation
// and assertion using macros. But macros are prone to double evaluation, if you
// pass function having side-effects. Passing a pure function or data is valid.
// Our macros check if the condition passed is unstable, this doesn't mean they
// will point out all mistakes, checking for stability once will catch the
// most obvious mistakes, nothing more.
//
// So ONLY pass variables to the V, VE and A macros!
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
// -----------
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
#begindef E(chirp, message, ...)
{
    if(chirp->_log != NULL) {
        char buf[1024];
        snprintf(
            buf,
            1024,
            "%s:%d " message,
            __FILE__,
            __LINE__,
            __VA_ARGS__
        );
        chirp->_log(buf, 1);
    } else {
        fprintf(
            stderr,
            "%s:%d Error: " message "\n",
            __FILE__,
            __LINE__,
            __VA_ARGS__
        );
    }
}
#enddef

#ifndef NDEBUG

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
//    :param ...: Variadic arguments (arbitrary arguments of arbitrary
//                quantity).
//
// .. code-block:: cpp
//
#   begindef V(chirp, condition, message, ...)
    {
        if(!(condition)) {
            if(chirp->_log != NULL) {
                char buf[1024];
                snprintf(
                    buf,
                    1024,
                    "%s:%d " message,
                    __FILE__,
                    __LINE__,
                    __VA_ARGS__
                );
                chirp->_log(buf, 1);
            } else {
                fprintf(
                    stderr,
                    "%s:%d " message "\n",
                    __FILE__,
                    __LINE__,
                    __VA_ARGS__
                );
                assert(condition);
                /* See the double evaluation chapter. */
                fprintf(stderr, "Bad assert: condition not stable\n");
                assert(0);
            }
            return CH_VALUE_ERROR;
        }
    }
#   enddef

// .. c:macro:: VE
//
//    Validates the given condition and reports a message when the condition is
//    not met in debug-/development-mode.
//
//    Be aware of :ref:`double-eval`
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
#   begindef VE(chirp, condition, message)
    {
        if(!(condition)) {
            if(chirp->_log != NULL) {
                char buf[1024];
                snprintf(
                    buf,
                    1024,
                    "%s:%d " message,
                    __FILE__,
                    __LINE__
                );
                chirp->_log(buf, 1);
            } else {
                fprintf(
                    stderr,
                    "%s:%d " message "\n",
                    __FILE__,
                    __LINE__
                );
                assert(condition);
                /* See the double evaluation chapter. */
                fprintf(stderr, "Bad assert: condition not stable\n");
                assert(0);
            }
            return CH_VALUE_ERROR;
        }
    }
#   enddef

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
#   begindef L(chirp, message, ...)
    {
        if(chirp->_log != NULL) {
            char buf[1024];
            snprintf(
                buf,
                1024,
                "%s:%d " message,
                __FILE__,
                __LINE__,
                __VA_ARGS__
            );
            chirp->_log(buf, 0);
        } else {
            fprintf(
                stderr,
                "%s:%d " message "\n",
                __FILE__,
                __LINE__,
                __VA_ARGS__
            );
        }
    }
#   enddef

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
//    :param chirp: Pointer to a chirp object.
//    :param ...: Variadic arguments (arbitrary arguments of arbitrary
//                quantity).
//
// .. code-block:: cpp
//
#   begindef A(condition, ...)
    {
        if(!(condition)) {
            fprintf(stderr, __VA_ARGS__);
            fprintf(stderr, "\n");
            assert(condition);
            /* See the double evaluation chapter. */
            fprintf(stderr, "Bad assert: condition not stable\n");
            assert(0);
        }
    }
#   enddef

#else //NDEBUG

// .. c:macro:: V
//
//    Validates the given condition and reports a message including arbitrary
//    given arguments when the condition is not met in release-mode.
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
//    :param ...: Variadic arguments (arbitrary arguments of arbitrary
//                quantity).
//
// .. code-block:: cpp
//
#   begindef V(chirp, condition, message, ...)
    {
        if(!(condition)) {
            if(chirp->_log != NULL) {
                char buf[1024];
                snprintf(
                    buf,
                    1024,
                    "%s:%d " message,
                    __FILE__,
                    __LINE__,
                    __VA_ARGS__
                );
                chirp->_log(buf, 1);
            } else {
                fprintf(
                    stderr,
                    "%s:%d " message "\n",
                    __FILE__,
                    __LINE__,
                    __VA_ARGS__
                );
            }
            return CH_VALUE_ERROR;
        }
    }
#   enddef

// .. c:macro:: VE
//
//    Validates the given condition and reports a message when the condition is
//    not met in release-mode.
//
//    Be aware of :ref:`double-eval`
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
#   begindef VE(chirp, condition, message)
    {
        if(!(condition)) {
            if(chirp->_log != NULL) {
                char buf[1024];
                snprintf(
                    buf,
                    1024,
                    "%s:%d " message,
                    __FILE__,
                    __LINE__
                );
                chirp->_log(buf, 1);
            } else {
                fprintf(
                    stderr,
                    "%s:%d " message "\n",
                    __FILE__,
                    __LINE__
                );
            }
            return CH_VALUE_ERROR;
        }
    }
#   enddef

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
//    Be aware of :ref:`double-eval`
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

// Windows compatiblity
// ====================
//
// .. code-block:: cpp

#ifdef _WIN32

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

// Generic functions
// =================
//
// .. code-block:: text
//
#begindef MINMAX_FUNCS(type)
    static
    ch_inline
    type
    max_##type(type a, type b) {
        return (a > b) ? a : b;
    }

    static
    ch_inline
    type
    min_##type(type a, type b) {
        return (a < b) ? a : b;
    }
#enddef


#endif //ch_common_h
