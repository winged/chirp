#include "quickcheck.h"

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
    ch_qc_init();
    ch_qc_gen gs[] = { ch_gen_odd };
    ch_qc_print ps[] = { ch_qc_print_int };
    return !ch_qc_for_all(ch_is_odd, 1, gs, ps, int);
}
