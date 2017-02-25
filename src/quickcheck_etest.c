// ==================
// Testing quickcheck
// ==================
//
// .. todo:: Document purpose
//
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
// .. code-block:: cpp
//
static int m = 0;

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

bool ch_is_ascii(ch_buf* data) {
    char* buf = ch_qc_args(char*, 0, char*);

    for(size_t i = 0; i < sizeof(buf); i++) {
        if(buf[i] > m) // Using global to shutup the compiler
            return 0;
    }
    return 1;
}

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

    m = 128;
    ch_qc_gen gs1[] = { ch_qc_gen_string };
    ch_qc_print ps1[] = { ch_qc_print_string };
    ret |= !ch_qc_for_all(ch_is_ascii, 1, gs1, ps1, int);
    return ret;
}
