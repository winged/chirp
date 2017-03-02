//.. _more-quickcheck-examples:
//
// ==================
// Testing quickcheck
// ==================
//
// Test quickcheck with a few simple examples.
//
// Project includes
// ================
//
// .. code-block:: cpp
//
#include "quickcheck.h"

// Test functions
// ==============
//
// Not documented on purpose.
//
// .. code-block:: cpp

void ch_gen_odd(ch_buf* data) {
    int i;
    ch_qc_gen_int((ch_buf*) &i);

    if (i % 2 == 0) {
        i++;
    }

    ch_qc_return(int, i);
}

bool ch_is_odd(ch_buf* data) {
    int n = ch_qc_args(int, 0, int);

    return n % 2 == 1;
}

bool ch_is_ascii_string(ch_buf* data) {
     ch_qc_mem_track_t* item = ch_qc_args(
        ch_qc_mem_track_t*,
        0,
        ch_qc_mem_track_t*
    );

    for(
            int i = 0;
            i < (item->count - 1);
            i++
    ) {
        if(item->data[i] < 1)
            return 0;
    }
    return item->data[(item->count * item->size) - 1] == 0;
}

// Runner
// ======

// .. c:function::
int
main(
    int argc,
    char *argv[]
)
//    :noindex:
//
//    Test quickcheck.
//
// .. code-block:: cpp
//
{
    (void)(argc); // I hate incomplete main headers;
    (void)(argv); // I hate incomplete main headers;
    int ret = 0;
    ch_qc_init();
    ch_qc_gen gs0[] = { ch_gen_odd };
    ch_qc_print ps0[] = { ch_qc_print_int };
    ret |= !ch_qc_for_all(ch_is_odd, 1, gs0, ps0, int);

    ch_qc_gen gs1[] = { ch_qc_gen_string };
    ch_qc_print ps1[] = { ch_qc_print_string };
    ret |= !ch_qc_for_all(
        ch_is_ascii_string,
        1,
        gs1,
        ps1,
        ch_qc_mem_track_t*
    );

    ch_qc_gen gs2[] = { ch_qc_gen_bytes };
    ch_qc_print ps2[] = { ch_qc_print_bytes };
    printf("The next test may (should) fail: ");
    ch_qc_for_all(
        ch_is_ascii_string,
        1,
        gs2,
        ps2,
        ch_qc_mem_track_t*
    ); // Just for memcheck
    return ret;
}
