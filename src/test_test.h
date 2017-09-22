// ===============================
// Common testing functions header
// ===============================
//
// Common testing helper functions
//
// Project includes
// ================
//
// .. code-block:: cpp
//
#include "libchirp/common.h"
#include "mpipe_test.h"
#include "mpack_test.h"

// .. c:function::
void
ch_tst_return_int(mpack_writer_t* writer, int val);
//
//    Write a integer back to the test driver
//
//    :param mpack_writer_t* writer: Mpack writer
//    :param int val: Value to return
//
// .. c:function::
void
ch_tst_return_float(mpack_writer_t* writer, float val);
//
//    Write a float back to the test driver
//
//    :param mpack_writer_t* writer: Mpack writer
//    :param float val: Value to return
//
