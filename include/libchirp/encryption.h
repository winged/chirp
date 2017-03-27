// ==========
// Encryption
// ==========
//
// ..todo:: Document purpose
//
// .. code-block:: cpp

#ifndef ch_libchirp_encryption_h
#define ch_libchirp_encryption_h

// Declarations
// ============

// .. c:function::
CH_EXPORT
ch_error_t
ch_en_openssl_init(void);
//
//    Initialize OpenSSL according to configuration.
//
//   :return: A chirp error. see: :c:type:`ch_error_t`
//   :rtype:  ch_error_t

// .. c:function::
CH_EXPORT
ch_error_t
ch_en_openssl_cleanup(void);
//
//    Cleanup OpenSSL.
//
//   :return: A chirp error. see: :c:type:`ch_error_t`
//   :rtype:  ch_error_t

// .. c:function::
CH_EXPORT
ch_error_t
ch_en_openssl_threading_cleanup(void);
//
//    DO NOT USE, unless you really really know what you are doing. Provided
//    for the rare case where your host application initializes openssl without
//    threading support, but you need threading. Chirp usually doesn't need
//    threading.
//
//    Cleanup openssl threading by setting destroying the locks and freeing
//    memory.
//

// .. c:function::
CH_EXPORT
ch_error_t
ch_en_openssl_threading_setup(void);
//
//    DO NOT USE, unless you really really know what you are doing. Provided
//    for the rare case where your host application initializes openssl without
//    threading support, but you need threading. Chirp usually doesn't need
//    threading.
//
//    Setup openssl threading by initializing the required locks and setting
//    the lock and the thread_id callbacks.
//

// .. c:function::
CH_EXPORT
void
ch_en_set_manual_openssl_init(void);
//
//    Manually initialize OpenSSL.
//
//    By default chirp will initialize openssl on the first instance of chirp
//    and cleanup openssl on the last instance of chirp. If for some
//    reason the count of chirp instances can drop to zero sometimes and you
//    do not want openssl to get uninitialized you can manually call
//    :c:func:`ch_en_openssl_init` before creating the first chirp instance and
//    :c:func:`ch_en_openssl_Cleanup` after closing the last chirp instance.
//
//    You can also initialize OpenSSL yourself if you have to or not initialize
//    it at all if your host application has already initialized openssl.
//

#endif //ch_libchirp_encryption_h
