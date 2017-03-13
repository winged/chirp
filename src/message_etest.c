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
#include "quickcheck.h"
#include "message_test.h"

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
    ch_start_cb_t init;
} ch_test_chirp_thread_t;

void
sent(ch_message_t* msg, int status, float load)
{
    (void)(status);
    (void)(load);
    printf("done\n");
    ch_chirp_close_ts(msg->chirp);
    ch_qc_free_mem();
}

void
send_message(ch_chirp_t* chirp)
{
    A(chirp->_init == CH_CHIRP_MAGIC, "Not a ch_chirp_t*");

    ch_message_t* msg = ch_test_gen_message(chirp);
    msg->port = 59732;
    ch_chirp_send(
            chirp,
            msg,
            sent
    );
}

void
sender_init_handler(ch_chirp_t* chirp)
{
    A(chirp->_init == CH_CHIRP_MAGIC, "Not a ch_chirp_t*");
    send_message(chirp);
    //ch_chirp_close_ts(chirp);
}

void
echo_init_handler(ch_chirp_t* chirp)
{
    A(chirp->_init == CH_CHIRP_MAGIC, "Not a ch_chirp_t*");
    ch_chirp_close_ts(chirp);
}

static
void
_ch_test_run_chirp(void* arg)
{
    ch_test_chirp_thread_t* args = (ch_test_chirp_thread_t*) arg;
    ch_chirp_t chirp;
    uv_loop_t loop;
    ch_config_t config;
    ch_chirp_config_init(&config);
    config.PORT = args->port;
    args->chirp = &chirp;
    config.CERT_CHAIN_PEM = "./cert.pem";
    config.DH_PARAMS_PEM = "./dh.pem";
    ch_loop_init(&loop);
    if(ch_chirp_init(
            &chirp,
            &config,
            &loop,
            args->init,
            NULL,
            NULL
    ) != CH_SUCCESS) {
        printf("ch_chirp_init error\n");
        return;
    }
    ch_chirp_set_auto_stop_loop(&chirp);
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
    // TODO test with and without encryption
    (void)(argc); // I hate incomplete main signatures
    (void)(argv); // I hate incomplete main signatures
    ch_test_chirp_thread_t sender_thread;
    //ch_test_chirp_thread_t echo_thread;
    sender_thread.port = 59731;
    sender_thread.init = sender_init_handler;
    /*echo_thread.port   = 59732;
    echo_thread.init = echo_init_handler;
    uv_thread_t echo;
    uv_thread_t sender;*/
    ch_libchirp_init();
    ch_en_set_manual_openssl_init(); // OpenSSL doesn't like to be initialized
                                     // from threads
    ch_en_openssl_init();
    //uv_thread_create(&echo, _ch_test_run_chirp, &echo_thread);
    sleep(1);
    _ch_test_run_chirp(&sender_thread);
    /*uv_thread_create(&sender, _ch_test_run_chirp, &sender_thread);
    uv_thread_join(&echo);
    uv_thread_join(&sender);*/
    ch_en_openssl_cleanup();
    ch_libchirp_cleanup();
}
