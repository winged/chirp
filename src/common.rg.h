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

struct ch_remote_s;
typedef struct ch_remote_s ch_remote_t;
struct ch_connection_s;
typedef struct ch_connection_s ch_connection_t;
struct ch_receipt_s;
typedef struct ch_receipt_s ch_receipt_t;


// Logging and assert macros
// =========================
//
// Colors
// ------
//
// .. code-block:: cpp

static const char*
_ch_lg_reset = "\x1B[0m";
static const char*
_ch_lg_err = "\x1B[1;31m";
static const char*
_ch_lg_colors[8] = {
    "\x1B[0;34m",
    "\x1B[0;32m",
    "\x1B[0;36m",
    "\x1B[0;33m",
    "\x1B[1;34m",
    "\x1B[1;32m",
    "\x1B[1;36m",
    "\x1B[1;33m",
};

static
void
__ch__silenence(void)
{
    (void)(_ch_lg_err);
    (void)(_ch_lg_reset);
    (void)(_ch_lg_colors);
}

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
// -----------
//
// .. c:macro:: ch_lg_get_id_m
//
//    Creates a id from 0-7 based on the identity of a chirp object.
//
//    :param chirp: Pointer to a chirp object.
//
// .. code-block:: cpp
//
#begindef _ch_lg_get_id_m(chirp)
    uint8_t __ch_log__id_ = (
        (uint8_t) chirp->_->identity[0]
    ) % 8
#enddef

// .. c:macro:: ch_lg_get_file_m
//
//    Creates the variable char* __ch_log__file_ pointing the file part of the
//    __FILE__ path.
//
// .. code-block:: cpp
//
#begindef _ch_lg_get_file_m()
    char* __ch_log__file_ = __FILE__;
    __ch_log__file_ = strrchr(__ch_log__file_, '/') + 1
#enddef

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
#begindef EC(chirp, message, clear, ...)
{
    _ch_lg_get_file_m();
    if(chirp->_log != NULL) {
        char __ch_v_buf_[1024];
        snprintf(
            __ch_v_buf_,
            1024,
            "%s:%d " message clear,
            __ch_log__file_,
            __LINE__,
            __VA_ARGS__
        );
        chirp->_log(__ch_v_buf_, 1);
    } else {
        _ch_lg_get_id_m(chirp);
        uv_mutex_lock(chirp->_log_lock);
        fprintf(
            stderr,
            "%s%02X%02X %15s:%4d Error: %s" message "\x1B[0m" clear "%s\n",
            _ch_lg_err,
            chirp->_->identity[0],
            chirp->_->identity[1],
            __ch_log__file_,
            __LINE__,
            _ch_lg_colors[__ch_log__id_],
            __VA_ARGS__,
            _ch_lg_reset
        );
        fflush(stderr);
        uv_mutex_unlock(chirp->_log_lock);
    }
}
#enddef
#define E(chirp, message, ...) EC(chirp, message, "",  __VA_ARGS__)

// .. c:macro:: TA
//
//    Test assert.
//
//    Validates the given condition and reports arbitrary arguments when the
//    condition is not met in debug-/development-mode.
//
//    Be aware of :ref:`double-eval`
//
//    The assert macro TA(condition, ...) behaves like printf and allows to
//    print a arbitrary arguments when the given assertion fails.
//
//    :param condition: A boolean condition to check.
//    :param ...: Variadic arguments for xprintf
//
// .. code-block:: cpp
//
#   begindef TA(condition, ...)
    {
        _ch_lg_get_file_m();
        if(!(condition)) {
            fprintf(
                stderr,
                "%s%20s:%4d ",
                _ch_lg_err,
                __ch_log__file_,
                __LINE__
            );
            fprintf(stderr, __VA_ARGS__);
            fprintf(stderr, "%s\n", _ch_lg_reset);
            fflush(stderr);
            exit(1);
        }
    }
#   enddef

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
//    :param ...: Variadic arguments for xprintf
//
// .. code-block:: cpp
//
#   begindef V(chirp, condition, ...)
    {
        if(!(condition)) {
            _ch_lg_get_file_m();
            if(chirp->_log != NULL) {
                size_t __ch_v_siz_ = 1024;
                size_t __ch_v_ret_;
                char __ch_v_buf_[__ch_v_siz_];
                char* __ch_v_xbuf_ = __ch_v_buf_;
                __ch_v_ret_ = snprintf(
                    __ch_v_xbuf_,
                    __ch_v_siz_,
                    "%s:%d ",
                    __ch_log__file_,
                    __LINE__
                );
                __ch_v_xbuf_ += __ch_v_ret_;
                __ch_v_siz_ -= __ch_v_ret_;
                snprintf(
                    __ch_v_xbuf_,
                    __ch_v_siz_,
                    __VA_ARGS__
                );
                chirp->_log(__ch_v_buf_, 1);
                return CH_VALUE_ERROR;
            } else {
                _ch_lg_get_id_m(chirp);
                uv_mutex_lock(chirp->_log_lock);
                fprintf(
                    stderr,
                    "%s%02X%02X %15s:%4d %s",
                    _ch_lg_err,
                    chirp->_->identity[0],
                    chirp->_->identity[1],
                    __ch_log__file_,
                    __LINE__,
                    _ch_lg_colors[__ch_log__id_]
                );
                fprintf(stderr, __VA_ARGS__);
                fprintf(stderr, "%s\n", _ch_lg_reset);
                fflush(stderr);
                uv_mutex_unlock(chirp->_log_lock); /* Just for consistency */
                assert(condition);
                /* See the double evaluation chapter. */
                fprintf(stderr, "Bad assert: condition not stable\n");
                assert(0);
            }
        }
    }
#   enddef

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
#   begindef LC(chirp, message, clear, ...)
    {
        _ch_lg_get_file_m();
        if(chirp->_log != NULL) {
            char buf[1024];
            snprintf(
                buf,
                1024,
                "%s:%d " message clear,
                __ch_log__file_,
                __LINE__,
                __VA_ARGS__
            );
            chirp->_log(buf, 0);
        } else {
            _ch_lg_get_id_m(chirp);
            uv_mutex_lock(chirp->_log_lock);
            fprintf(
                stderr,
                "%s%02X%02X%s %15s:%4d%s " message "\x1B[0m" clear "%s\n",
                _ch_lg_colors[__ch_log__id_],
                chirp->_->identity[0],
                chirp->_->identity[1],
                _ch_lg_reset,
                __ch_log__file_,
                __LINE__,
                _ch_lg_colors[__ch_log__id_],
                __VA_ARGS__,
                _ch_lg_reset
            );
            fflush(stderr);
            uv_mutex_unlock(chirp->_log_lock);
        }
    }
#   enddef
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
#   begindef A(condition, ...)
    {
        if(!(condition)) {
            fprintf(stderr, "%20s", _ch_lg_err);
            fprintf(stderr, __VA_ARGS__);
            fprintf(stderr, "%s\n", _ch_lg_reset);
            assert(condition);
            /* See the double evaluation chapter. */
            fprintf(stderr, "Bad assert: condition not stable\n");
            fflush(stderr);
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
//    :param ...: Variadic arguments for xprintf
//
// .. code-block:: cpp
//
#   begindef V(chirp, condition, ...)
    {
        if(!(condition)) {
            _ch_lg_get_file_m();
            if(chirp->_log != NULL) {
                size_t __ch_v_siz_ = 1024;
                size_t __ch_v_ret_;
                char __ch_v_buf_[siz];
                char* __ch_v_xbuf_ = __ch_v_buf_;
                __ch_v_ret_ = snprintf(
                    __ch_v_xbuf_,
                    __ch_v_siz_,
                    "%s:%d ",
                    __ch_log__file_,
                    __LINE__
                );
                __ch_v_xbuf_ += __ch_v_ret_;
                __ch_v_siz_ -= __ch_v_ret_;
                __ch_v_ret_ = snprintf(__VA_ARGS__);
                chirp->_log(__ch_v_buf_, 1);
            } else {
                fprintf(
                    stderr,
                    "%s:%d", /* No coloring because its not debug mode */
                    __ch_log__file_,
                    __LINE__
                );
                fprintf(stderr, __VA_ARGS__);
                fprintf(stderr, "\n");
                fflush(stderr);
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
