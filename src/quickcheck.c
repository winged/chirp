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

// Definitions
// ===========
//
// .. code-block:: cpp
//

static bool QC_INITIALIZED = false;

void qc_init(void) {
    srand((unsigned int) time(NULL));
}

void gen_bool(/*@out@*/ blob const data) {
    bool b = rand() % 2 == 0;

    qc_return(bool, b);
}

void gen_int(/*@out@*/ blob const data) {
    int i = rand();

    qc_return(int, i);
}

void gen_char(/*@out@*/ blob const data) {
    char c = (char)(rand() % 128);

    qc_return(char, c);
}

void _gen_array(/*@out@*/ blob const data, gen const g, size_t const size) {
    int len = rand() % 100;
    
    blob arr = malloc((size_t) len * size);
    
    size_t i;
    
    for (i = 0; i < (size_t) len; i++) {
        g(arr + i * size);
    }
    
    qc_return(blob, arr);
}

void gen_string(/*@out@*/ blob const data) {
    char *s;
    
    gen_array(&s, gen_char, char);
    
    qc_return(char *, s);
}

void print_bool(blob const data) {
    bool b = qc_args(bool, 0, bool);

    printf("%s", b ? "true" : "false");
}

void print_int(blob const data) {
    int i = qc_args(int, 0, int);

    printf("%d", i);
}

void print_char(blob const data) {
    char c = qc_args(char, 0, char);

    printf("\'%c\'", c);
}

void print_string(blob const data) {
    char *s = qc_args(char *, 0, char *);

    printf("%s", s);
}


bool _for_all(
        prop const property,
        size_t const arglen,
        gen const gs[],
        print const ps[],
        size_t const max_size
) {
    size_t i, j;
    blob values;
    
    // Because GC_MALLOC will segfault if GC_INIT() is not called beforehand.
    if (!QC_INITIALIZED) {
        printf("*** Error: Run qc_init() before calling for_all().\n");
        return false;
    }
    
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
            
            free(values);
            return false;
        }
    }
    
    printf("+++ OK, passed 100 tests.\n");
    
    free(values);
    return true;
}
