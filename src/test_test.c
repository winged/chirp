// ========================
// Common testing functions
// ========================
//
// Commont testing helper functions
//
// Project includes
// ================
//
// .. code-block:: cpp
//
#include "test_test.h"

// .. c:function::
void
ch_tst_return_int(mpack_writer_t* writer, int val)
//    :noindex:
//
//    see: :c:func:`ch_tst_return_int`
//
// .. code-block:: cpp
//
{
    mpack_start_array(writer, 1);
    mpack_write_int(writer, val);
    mpack_finish_array(writer);
}

// .. c:function::
void
ch_tst_return_float(mpack_writer_t* writer, float val)
//    :noindex:
//
//    see: :c:func:`ch_tst_return_float`
//
// .. code-block:: cpp
//
{
    mpack_start_array(writer, 1);
    mpack_write_float(writer, val);
    mpack_finish_array(writer);
}
