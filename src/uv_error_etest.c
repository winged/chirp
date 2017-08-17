// =======================
// Converting libuv errors
// =======================
//
// First argument is a libuv error number like -14
//
// System includes
// ===============
//
// .. code-block:: cpp
//
#include <uv.h>
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char *argv[]) {
    int err;
    if(argc < 2) {
        printf("The error number should be the first argument\n");
        return 1;
    }
    err = strtol(argv[1], NULL, 10);
    if(errno != 0) {
        printf("The argument is not a number\n");
        return 1;
    }
    printf("%s\n", uv_strerror(err));
    return 0;
}
