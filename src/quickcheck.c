// ==========
// Quickcheck
// ==========
//
// .. code-block:: cpp
//
//
// Project includes
// ================
//
// .. code-block:: cpp
//
#include "quickcheck.h"

// System includes
// ===============
//
// .. code-block:: cpp
//
#include <stdlib.h>
#include <stdio.h>
#include <time.h>

// Declarations
// ============
//
// .. c:function::
static
void
_ch_qc_free_mem(void);
//
//    Free memory allocations made during quickcheck test.
//
// .. c:var:: ch_qc_mem_track_t* _ch_qc_mem_track
//
//    List of memory allocations during current quickcheck test.
//
// .. code-block:: cpp

static ch_qc_mem_track_t* _ch_qc_mem_track = NULL;

// Definitions
// ===========
//
// Sglib Prototypes
// ----------------
//
// .. code-block:: cpp

SGLIB_DEFINE_LIST_FUNCTIONS( // NOCOV
    ch_qc_mem_track_t,
    CH_QC_MEM_TRACK_COMPARATOR,
    next
)
//
// .. c:function::
int
_ch_qc_for_all(
        ch_qc_prop property,
        size_t arglen,
        ch_qc_gen gs[],
        ch_qc_print ps[],
        size_t max_size
)
//    :noindex:
//
//    see: :c:func:`_ch_qc_for_all`
//
// .. code-block:: cpp
//
{
    size_t i, j;
    ch_buf* values;

    values = malloc(arglen * max_size);

    for (i = 0; i < 100; i++) {
        for (j = 0; j < arglen; j++) {
            gs[j](values + j * max_size);
        }

        bool holds = property(values);

        if (!holds) {
            printf("*** Failed!\n");

            for (j = 0; j < arglen; j++) {
                ps[j](values + j * max_size);
                printf("\n");
            }
            _ch_qc_free_mem();
            free(values);
            return 0;
        }
    }

    printf("+++ OK, passed 100 tests.\n");

    _ch_qc_free_mem();
    free(values);
    return 1;
}
//
// .. c:function::
static
void
_ch_qc_free_mem(void)
//    :noindex:
//
//    see: :c:func:`_ch_qc_free_mem`
//
// .. code-block:: cpp
//
{
    ch_qc_mem_track_t* t;
    struct sglib_ch_qc_mem_track_t_iterator it;
    for(
            t = sglib_ch_qc_mem_track_t_it_init(
                &it,
                _ch_qc_mem_track
            );
            t != NULL;
            t = sglib_ch_qc_mem_track_t_it_next(&it)
    ) {
        free(t->mem);
        free(t);
    }
}

// .. c:function::
void
_ch_qc_gen_array(
        ch_buf* data,
        ch_qc_gen g,
        size_t size
)
//    :noindex:
//
//    see: :c:func:`_ch_qc_gen_array`
//
// .. code-block:: cpp
//
{
    int len = rand() % 100;

    ch_buf* arr = malloc((size_t) len * size);
    ch_qc_mem_track_t* item = malloc(sizeof(ch_qc_mem_track_t));
    item->mem = arr;
    sglib_ch_qc_mem_track_t_add(&_ch_qc_mem_track, item);

    size_t i;

    for (i = 0; i < (size_t) len; i++) {
        g(arr + i * size);
    }

    ch_qc_return(ch_buf*, arr);
}

// .. c:function::
void
ch_qc_gen_bool(ch_buf* data)
//    :noindex:
//
//    see: :c:func:`ch_qc_gen_bool`
//
// .. code-block:: cpp
//
{
    int b = rand() % 2 == 0;

    ch_qc_return(int, b);
}

// .. c:function::
void
ch_qc_gen_char(ch_buf* data)
//    :noindex:
//
//    see: :c:func:`ch_qc_gen_char`
//
// .. code-block:: cpp
//
{
    char c = (char)(rand() % 128);

    ch_qc_return(char, c);
}

// .. c:function::
void
ch_qc_gen_int(ch_buf* data)
//    :noindex:
//
//    see: :c:func:`ch_qc_gen_int`
//
// .. code-block:: cpp
//
{
    int i = rand();

    ch_qc_return(int, i);
}

// .. c:function::
void
ch_qc_gen_string(ch_buf* data)
//    :noindex:
//
//    see: :c:func:`ch_qc_gen_string`
//
// .. code-block:: cpp
//
{
    char *s;

    ch_qc_gen_array((ch_buf*) &s, ch_qc_gen_char, char);

    ch_qc_return(char *, s);
}

// .. c:function::
void
ch_qc_init(void)
//    :noindex:
//
//    see: :c:func:`ch_qc_init`
//
// .. code-block:: cpp
//
{
    srand((unsigned int) time(NULL));
}

// .. c:function::
void
ch_qc_print_bool(ch_buf* data)
//    :noindex:
//
//    see: :c:func:`ch_qc_print_bool`
//
// .. code-block:: cpp
//
{
    int b = ch_qc_args(int, 0, int);

    printf("%s", b ? "true" : "false");
}

// .. c:function::
void
ch_qc_print_char(ch_buf* data)
//    :noindex:
//
//    see: :c:func:`ch_qc_print_char`
//
// .. code-block:: cpp
//
{
    char c = ch_qc_args(char, 0, char);

    printf("\'%c\'", c);
}

// .. c:function::
void
ch_qc_print_int(ch_buf* data)
//    :noindex:
//
//    see: :c:func:`ch_qc_print_int`
//
// .. code-block:: cpp
//
{
    int i = ch_qc_args(int, 0, int);

    printf("%d", i);
}

// .. c:function::
void
ch_qc_print_string(ch_buf* data)
//    :noindex:
//
//    see: :c:func:`ch_qc_print_string`
//
// .. code-block:: cpp
//
{
    char *s = ch_qc_args(char*, 0, char*);

    printf("%s", s);
}
