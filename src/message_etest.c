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
#include "quickcheck_test.h"
#include "message_test.h"

// System includes
// ===============
//
// .. code-block:: cpp
//
#include <unistd.h>

// Declarations
// ============
//
// .. code-block:: cpp
//

#define ch_tr_other_chirp(x) \
    ( \
        (ch_test_chirp_thread_t*)(x)->user_data \
    )->other->chirp

#define PORT_SENDER     59731
#define PORT_ECHO       59732

static
void
ch_send_message(ch_chirp_t* chirp);

// Test functions
// ==============
//
// .. code-block:: cpp
//
int _msg_send_count = 0;
static ch_message_t _msg_send;
int _msg_echo_count = 0;
static ch_message_t _msg_echo;
static char _data[] = "hello";

struct ch_test_chirp_thread_s;
typedef struct ch_test_chirp_thread_s ch_test_chirp_thread_t;

struct ch_test_chirp_thread_s {
    int port;
    ch_start_cb_t init;
    ch_test_chirp_thread_t* other;
    ch_chirp_t* chirp;
};

static
void
ch_simple_msg(ch_chirp_t* chirp, ch_message_t* msg)
{
    ch_msg_init(chirp, msg);
    ch_msg_set_address(
        msg,
        CH_IPV4,
        "127.0.0.1",
        PORT_ECHO
    );
    msg->data = _data;
    msg->data_len = strnlen(_data, sizeof(_data));
}

static
void
ch_echo(ch_message_t* msg, int status, float load)
{
    (void)(status);
    (void)(load);
    (void)(msg);
    //ch_qc_free_mem();
    _msg_echo_count += 1;
}

static
void
ch_sent(ch_message_t* msg, int status, float load)
{
    (void)(status);
    (void)(load);
    (void)(msg);
    //ch_qc_free_mem();
    _msg_send_count += 1;
    if(_msg_send_count < 10) {
        ch_send_message(msg->chirp);
    } else {
        /* TODO: Insted of sleep, actually wait for the last echoed message to
         * arrive.
         */;
        sleep(1);
        ch_chirp_close_ts(ch_tr_other_chirp(msg->chirp));
        ch_chirp_close_ts(msg->chirp);
    }
}

static
void
ch_recv_echo_message_cb(ch_chirp_t* chirp, ch_message_t* msg)
{
    A(chirp->_init == CH_CHIRP_MAGIC, "Not a ch_chirp_t*");
    A(msg != NULL, "Not a ch_message_t*");
    if(!(msg->message_type & CH_MSG_REQ_ACK)) {
        // abort if message was an ACK
        return;
    }
    ch_simple_msg(chirp, &_msg_echo);
    _msg_echo.port = PORT_SENDER;
    /* TODO Send a echo message
     * ch_chirp_send(
     *         chirp,
     *         &_msg_echo,
     *         ch_echo
     * );
     */
}

static
void
ch_send_message(ch_chirp_t* chirp)
{
    A(chirp->_init == CH_CHIRP_MAGIC, "Not a ch_chirp_t*");

    int simple = ch_qc_tgen_bool();
    simple = 1; // TODO remove

    if(simple) {
        ch_simple_msg(chirp, &_msg_send);
        _msg_send.port = PORT_ECHO;
        ch_chirp_send(
                chirp,
                &_msg_send,
                ch_sent
        );
    } else {
        ch_message_t* msg = ch_test_gen_message(chirp);
        msg->port = PORT_ECHO;
        ch_chirp_send(
                chirp,
                msg,
                ch_sent
        );
    }
}

static
void
sender_init_handler(ch_chirp_t* chirp)
{
    A(chirp->_init == CH_CHIRP_MAGIC, "Not a ch_chirp_t*");
    ch_send_message(chirp);
}

static
void
echo_init_handler(ch_chirp_t* chirp)
{
    A(chirp->_init == CH_CHIRP_MAGIC, "Not a ch_chirp_t*");
    ch_chirp_register_recv_cb(chirp, ch_recv_echo_message_cb);
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
    args->chirp = &chirp;
    chirp.user_data = args;
    ch_chirp_set_auto_stop_loop(&chirp);
    ch_run(&loop); /* Will block till loop stops */
    ch_loop_close(&loop);
}


// Runner
// ======
//
// .. code-block:: cpp

int
main()
{
    // TODO test with and without encryption
    ch_test_chirp_thread_t sender_thread;
    ch_test_chirp_thread_t echo_thread;
    sender_thread.other = &echo_thread;
    echo_thread.other = &sender_thread;
    sender_thread.port = PORT_SENDER;
    sender_thread.init = sender_init_handler;
    echo_thread.port   = PORT_ECHO;
    echo_thread.init   = echo_init_handler;
    uv_thread_t echo;
    uv_thread_t sender;
    ch_libchirp_init();
    uv_thread_create(&echo, _ch_test_run_chirp, &echo_thread);
    sleep(1);
    uv_thread_create(&sender, _ch_test_run_chirp, &sender_thread);
    uv_thread_join(&echo);
    uv_thread_join(&sender);
    ch_libchirp_cleanup();
}
