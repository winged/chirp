// =============
// Message etest
// =============
//
// Testing sending/receiving messages
//
// Project includes
// ================
//
// .. code-block:: cpp
//
#include "libchirp.h"
#include "common.h"

// System includes
// ===============
//
// .. code-block:: cpp
//
#include <unistd.h>

// Test functions
// ==============
//
// Not documented on purpose.
//
// .. code-block:: cpp

typedef struct ch_test_chirp_thread_s {
    ch_chirp_t* chirp;
    int port;
    uv_async_cb init;
} ch_test_chirp_thread_t;

void
sender_init_handler(uv_async_t* handle)
{
    ch_chirp_t* chirp = handle->data;
    A(chirp->_init == CH_CHIRP_MAGIC, "Not a ch_chirp_t*");
    //ch_chirp_close_ts(chirp);
}

void
echo_init_handler(uv_async_t* handle)
{
    ch_chirp_t* chirp = handle->data;
    A(chirp->_init == CH_CHIRP_MAGIC, "Not a ch_chirp_t*");
    //ch_chirp_close_ts(chirp);
}

static
void
_ch_test_run_chirp(void* arg)
{
    ch_test_chirp_thread_t* args = (ch_test_chirp_thread_t*) arg;
    uv_async_t init_call;
    ch_chirp_t chirp;
    uv_loop_t loop;
    ch_config_t config;
    ch_chirp_config_init(&config);
    config.PORT = args->port;
    args->chirp = &chirp;
    config.CERT_CHAIN_PEM = "./cert.pem";
    config.DH_PARAMS_PEM = "./dh.pem";
    ch_loop_init(&loop);
    if(uv_async_init(&loop, &init_call, args->init) < 0) {
        printf("uv_async_init error\n");
        return;
    }
    init_call.data = &chirp;
    if(ch_chirp_init(&chirp, &config, &loop, NULL, NULL) != CH_SUCCESS) {
        printf("ch_chirp_init error\n");
        return;
    }
    ch_chirp_set_auto_stop_loop(&chirp);
    uv_async_send(&init_call);
    ch_run(&loop); // Will block till loop stops
    ch_loop_close(&loop);
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
    (void)(argc); // I hate incomplete main signatures
    (void)(argv); // I hate incomplete main signatures
    ch_test_chirp_thread_t sender_thread;
    ch_test_chirp_thread_t echo_thread;
    sender_thread.port = 59731;
    sender_thread.init = sender_init_handler;
    echo_thread.port   = 59732;
    echo_thread.init = echo_init_handler;
    uv_thread_t echo;
    uv_thread_t sender;
    ch_libchirp_init();
    ch_en_set_manual_openssl_init(); // OpenSSL doesn't like to be initialized
                                     // from threads
    ch_en_openssl_init();
    uv_thread_create(&echo, _ch_test_run_chirp, &echo_thread);
    sleep(1);
    uv_thread_create(&sender, _ch_test_run_chirp, &sender_thread);
    uv_thread_join(&echo);
    uv_thread_join(&sender);
    ch_en_openssl_cleanup();
    ch_libchirp_cleanup();
}
