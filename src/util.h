// ===========
// Util Header
// ===========
//
// Common utility functions.
//
// .. code-block:: cpp
//
#ifndef ch_util_h
#define ch_util_h

// Project includes
// ================
//
// .. code-block:: cpp
//
#include "common.h"
#include "libchirp/callbacks.h"

// Declarations
// ============

// .. c:function::
void*
ch_alloc(size_t size);
//
//    Allocate fixed amount of memory.
//
//    :param size_t size: The amount of memory in bytes to allocate

// .. c:function::
void
ch_free(
        void* buf
);
//
//    Free a memory handle.
//
//    :param void* buf: The handle to free.

// .. c:function::
void*
ch_realloc(
        void*  buf,
        size_t size
);
//
//    Resize allocated memory.
//
//    :param void* buf:    The handle to resize.
//    :param size_t size:  The new size of the memory in bytes.

// Definitions
// ===========

// .. c:function::
static
ch_inline
void
ch_bytes_to_hex(uint8_t* bytes, size_t bytes_size, char* str, size_t str_size)
//
//    Convert a bytes array to a hex string.
//
//    :param uint8_t* bytes:     Bytes to convert.
//    :param size_t bytes_size:  Length of the bytes to convert.
//    :param char* str:          Destination string.
//    :param size_t str_size:    Length of the buffer to write the string to.
//
// .. code-block:: cpp
//
{
    size_t i;
    A(bytes_size * 2 + 1 <= str_size, "Not enough space for string");
    for(i = 0; i < bytes_size; i++)
    {
            snprintf(str, 3, "%02X", bytes[i]);
            str += 2;
    }
    *str = 0;
}

// .. c:function::
static
ch_inline
int
ch_is_local_addr(ch_text_address_t* addr)
//
//    Check if an address is either 127.0.0.1 or ::1
//
//    :param ch_text_address_t* addr: Address to check
//
// .. code-block:: cpp
//
{
    /* TODO move to util.c, create _ch_always_encrypt and change this code
     * accordingly.
     */
    return 0;
    return (
        strncmp("::1", addr->data, sizeof(ch_text_address_t)) ||
        strncmp("127.0.0.1", addr->data, sizeof(ch_text_address_t))
    );
}

// .. c:function::
static
ch_inline
int
ch_msb32(uint32_t x)
//
//    Get the most significant bit set of a set of bits.
//
//    :param uint32_t x:  The set of bits.
//
//    :return:            the most significant bit set.
//    :rtype:             uint32_t
//
// .. code-block:: cpp
//
{
    static const uint32_t bval[] =
    {0,1,2,2,3,3,3,3,4,4,4,4,4,4,4,4};

    uint32_t r = 0;
    if (x & 0xFFFF0000) { r += 16/1; x >>= 16/1; }
    if (x & 0x0000FF00) { r += 16/2; x >>= 16/2; }
    if (x & 0x000000F0) { r += 16/4; x >>= 16/4; }
    return r + bval[x];
}

// .. c:function::
static
ch_inline
void
ch_random_ints_as_bytes(uint8_t* bytes, size_t len)
//
//    Fill in random ints efficiently.
//    The parameter ``len`` MUST be multiple of four.
//
//    Thank you windows, for making this really complicated.
//
//    :param uint8_t* bytes:  The buffer to fill the bytes into.
//    :param size_t  len:     The length of the buffer.
//
// .. code-block:: cpp
//
{
    size_t i;
    int tmp_rand;
    A(len % 4 == 0, "len must be multiple of four");
#ifdef _WIN32
#   if RAND_MAX < 16384 || INT_MAX < 16384 // 2**14
#       error Seriously broken compiler or platform
#   else // RAND_MAX < 16384 || INT_MAX < 16384
        for(i = 0; i < len; i += 2) {
            tmp_rand = rand();
            memcpy(bytes + i, &tmp_rand, 2);
        }
#   endif // RAND_MAX < 16384 || INT_MAX < 16384
#else // _WIN32
#   if RAND_MAX < 1073741824 || INT_MAX < 1073741824 // 2**30
#       ifdef CH_ACCEPT_STRANGE_PLATFORM
            /* WTF, fallback platform */
            (void)(tmp_rand);
            for(i = 0; i < len; i++) {
                bytes[i] = ((unsigned int) rand()) % 256;
            }
#       else // ACCEPT_STRANGE_PLATFORM
            // cppcheck-suppress preprocessorErrorDirective
#           error Unexpected RAND_MAX / INT_MAX, define \
                CH_ACCEPT_STRANGE_PLATFORM
#       endif // ACCEPT_STRANGE_PLATFORM
#   else // RAND_MAX < 1073741824 || INT_MAX < 1073741824
        /* Tested: this is 4 times faster*/
        for(i = 0; i < len; i += 4) {
            tmp_rand = rand();
            memcpy(bytes + i, &tmp_rand, 4);
        }
#   endif // RAND_MAX < 1073741824 || INT_MAX < 1073741824
#endif // _WIN32
}

// .. c:function::
static
ch_inline
ch_error_t
ch_uv_error_map(int error)
//
//    Map common libuv errors to chirp errors.
//
//    :param int error:  Libuv error.
//
//    :return:           a chirp error.
//    :rtype:            ch_error_t
//
// .. code-block:: cpp
//
{
    switch(error) {
        case(0):
            return CH_SUCCESS;
        case(UV_EADDRINUSE):
            return CH_EADDRINUSE;
        case(UV_ENOTCONN):
        case(UV_EINVAL):
            return CH_VALUE_ERROR;
        default:
            return CH_UV_ERROR;
    }
}

#endif // ch_util_h
