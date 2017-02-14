// ===================
// Testing basic chirp
// ===================
//
// .. code-block:: cpp

#include "libchirp.h"


// .. c:function::
int
main(
    int argc,
    char *argv[]
)
//    :noindex:
//
//    Run chirp
//
// .. code-block:: cpp

{
    (void)(argc); // I hate incomplete main headers;
    (void)(argv); // I hate incomplete main headers;
    ch_chirp_t chirp;
    uv_loop_t loop;
    ch_config_t config;
    ch_chirp_config_init(&config);
    config.CERT_CHAIN_PEM = "./chirp/cert.pem";
    config.DH_PARAMS_PEM = "./chirp/dh.pem";
    ch_loop_init(&loop);
    if(ch_chirp_init(&chirp, &config, &loop, NULL) != CH_SUCCESS) {
        printf("ch_chirp_init error\n");
        return 1;
    }
    ch_chirp_set_auto_stop(&chirp);
    ch_run(&loop);
    return ch_loop_close(&loop);
}
