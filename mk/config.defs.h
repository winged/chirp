#ifndef ch_global_config_h
#define ch_global_config_h

// Version of chirp
//
// .. code-block:: cpp
//
#define CH_VERSION "XVERSIONX"

// Minimal buffersize we require when allocating for libuv
//
// .. code-block:: cpp

#define CH_LIB_UV_MIN_BUFFER 1024

// Preallocated buffer size for header. If the size is to small the buffer gets
// allocated using ch_alloc().
//
// .. code-block:: cpp

#define CH_BF_PREALLOC_HEADER 32

// Preallocated buffer size for actor. If the size is to small the buffer gets
// allocated using ch_alloc().
//
// .. code-block:: cpp

#define CH_BF_PREALLOC_ACTOR 256

// Preallocated buffer size for data. If the size is to small the buffer gets
// allocated using ch_alloc().
//
// .. code-block:: cpp

#define CH_BF_PREALLOC_DATA 512

////#define CH_CN_PRINT_CIPHERS

#endif //ch_global_config_h
