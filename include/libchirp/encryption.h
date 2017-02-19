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
extern
ch_error_t
ch_en_openssl_init(void);
//
//    Initialize OpenSSL according to configuration.
//
//   :return: A chirp error. see: :c:type:`ch_error_t`
//   :rtype:  ch_error_t

// .. c:function::
extern
ch_error_t
ch_en_openssl_uninit(void);
//
//    Uninitialize OpenSSL.
//
//   :return: A chirp error. see: :c:type:`ch_error_t`
//   :rtype:  ch_error_t

// .. c:function::
extern
void
ch_en_set_manual_openssl_init(void);
//
//    Manually initialize OpenSSL.
//
//    By default chirp will initialize openssl on the first instance of chirp
//    and uninitialize openssl on the last instance of chirp. If for some
//    reason the count of chirp instances can drop to zero sometimes and you
//    do not want openssl to get uninitialized you can manually call
//    :c:func:`ch_en_openssl_init` before creating the first chirp instance and
//    :c:func:`ch_en_openssl_uninit` after closing the last chirp instance.
//
//    You can also initialize OpenSSL yourself if you have to.
//

#endif //ch_libchirp_encryption_h
