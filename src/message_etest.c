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
#include "chirp.h"

// System includes
// ===============
//
// .. code-block:: cpp
//
#include <unistd.h>
#include <getopt.h>
#include <assert.h>

// Declarations
// ============
//
// .. code-block:: cpp
//

typedef enum {
    CH_TST_ALWAYS_ENCRYPT = 1000,
    CH_TST_MESSAGE_COUNT  = 1001,
    CH_TST_BUFFER_SIZE    = 1002,
    CH_TST_TIMEOUT        = 1003,
    CH_TST_HELP           = 1004,
} ch_tst_args_t;

typedef int (*ch_tst_chirp_send_t)(
        ch_chirp_t* chirp,
        ch_message_t* msg,
        ch_send_cb_t send_cb
);

#define ch_tr_other_chirp(x) \
    ( \
        (ch_tst_chirp_thread_t*)(x)->user_data \
    )->other->chirp

#define PORT_SENDER     59731
#define PORT_ECHO       59732

static
void
_ch_tst_send_message(ch_chirp_t* chirp);

// Definitions
//
// .. code-block:: cpp
//
MINMAX_FUNCS(int)

// Test functions
// ==============
//
// .. code-block:: cpp
//
static int _ch_tst_msg_send_count = 0;
static ch_message_t _ch_tst_msg;
static int _msg_echo_count = 0;
static ch_message_t _ch_tst_msg_echo;
static char _data[] = "hello";
static int _ch_tst_buffer_size = 0;
static float _ch_tst_timeout = 5.0;
static int _ch_tst_message_count = 20;

struct ch_tst_chirp_thread_s;
typedef struct ch_tst_chirp_thread_s ch_tst_chirp_thread_t;

struct ch_tst_chirp_thread_s {
    int port;
    ch_start_cb_t init;
    ch_tst_chirp_thread_t* other;
    ch_chirp_t* chirp;
};

static
void
_ch_tst_simple_msg(ch_chirp_t* chirp, ch_message_t* msg)
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
_ch_tst_echo_cb(ch_message_t* msg, int status, float load)
{
    (void)(status);
    (void)(load);
    (void)(msg);
    _msg_echo_count += 1;
}

static
void
_ch_tst_sent_cb(ch_message_t* msg, int status, float load)
{
    (void)(status);
    (void)(load);
    (void)(msg);
    ch_chirp_t* chirp = msg->chirp;
    ch_qc_free_mem();
    _ch_tst_msg_send_count += 1;
    if(_ch_tst_msg_send_count < _ch_tst_message_count) {
        _ch_tst_send_message(chirp);
    } else {
        /* TODO: Insted of sleep, actually wait for the last echoed message to
         * arrive.
         */;
        sleep(1);
        ch_chirp_close_ts(ch_tr_other_chirp(chirp));
        ch_chirp_close_ts(chirp);
    }
}

static
void
_ch_tst_recv_echo_message_cb(ch_chirp_t* chirp, ch_message_t* msg)
{
    A(chirp->_init == CH_CHIRP_MAGIC, "Not a ch_chirp_t*");
    A(msg != NULL, "Not a ch_message_t*");
    A(!(msg->type & CH_MSG_ACK), "ACK should not call callback");
    if(memcmp(msg->data, "hello", min_int(5, msg->data_len)) == 0) {
        L(
            chirp,
            "Echo received hello%s", ""
        )
    } else {
        assert(ch_tst_check_pattern(msg->header, msg->header_len));
        assert(ch_tst_check_pattern(msg->actor, msg->actor_len));
        assert(ch_tst_check_pattern(msg->data, msg->data_len));
        L(
            chirp,
            "Echo received a message%s", ""
        )
    }
    memcpy(&_ch_tst_msg_echo, msg, sizeof(ch_message_t));
    /* TODO Send an echo message
     * ch_chirp_send(
     *         chirp,
     *         &_ch_tst_msg_echo,
     *         _ch_tst_echo_cb
     * );
     */
}

static
void
_ch_tst_send_message(ch_chirp_t* chirp)
{
    A(chirp->_init == CH_CHIRP_MAGIC, "Not a ch_chirp_t*");

    int simple = ch_qc_tgen_bool();
    int use_ts = ch_qc_tgen_bool();

    ch_tst_chirp_send_t send_func;
    if(use_ts)
        send_func = ch_chirp_send_ts;
    else
        send_func = ch_chirp_send;

    if(simple) {
        _ch_tst_simple_msg(chirp, &_ch_tst_msg);
        _ch_tst_msg.port = PORT_ECHO;
        send_func(
                chirp,
                &_ch_tst_msg,
                _ch_tst_sent_cb
        );
    } else {
        ch_message_t* msg = ch_tst_gen_message(chirp);
        assert(ch_tst_check_pattern(msg->header, msg->header_len));
        assert(ch_tst_check_pattern(msg->actor, msg->actor_len));
        assert(ch_tst_check_pattern(msg->data, msg->data_len));
        msg->port = PORT_ECHO;
        send_func(
                chirp,
                msg,
                _ch_tst_sent_cb
        );
    }
}

static
void
_ch_tst_sender_init_handler(ch_chirp_t* chirp)
{
    A(chirp->_init == CH_CHIRP_MAGIC, "Not a ch_chirp_t*");
    _ch_tst_send_message(chirp);
}

static
void
_ch_tst_echo_init_handler(ch_chirp_t* chirp)
{
    A(chirp->_init == CH_CHIRP_MAGIC, "Not a ch_chirp_t*");
    ch_chirp_register_recv_cb(chirp, _ch_tst_recv_echo_message_cb);
}

static
void
_ch_tst_run_chirp(void* arg)
{
    ch_tst_chirp_thread_t* args = (ch_tst_chirp_thread_t*) arg;
    ch_chirp_t chirp;
    uv_loop_t loop;
    ch_config_t config;
    ch_chirp_config_init(&config);
    config.BUFFER_SIZE    = _ch_tst_buffer_size;
    config.TIMEOUT        = _ch_tst_timeout;
    config.REUSE_TIME     = _ch_tst_timeout * 15;
    config.PORT           = args->port;
    config.CERT_CHAIN_PEM = "./cert.pem";
    config.DH_PARAMS_PEM  = "./dh.pem";
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
//
static
void
print_help(struct option long_options[])
{
    printf("Usage:\n\n");
    while(long_options->name != NULL) {
        printf("    --%s ", long_options->name);
        if(long_options->has_arg == required_argument)
            printf("arg");
        else if(long_options->has_arg == optional_argument)
            printf("[arg]");
        printf("\n");
        long_options++;
    }
}

int
main(int argc, char *argv[])
{
    int c;
    int option_index;
    static struct option long_options[] = {
        {"always-encrypt", no_argument,       0, CH_TST_ALWAYS_ENCRYPT },
        {"message-count",  required_argument, 0, CH_TST_MESSAGE_COUNT },
        {"timeout",        required_argument, 0, CH_TST_TIMEOUT },
        {"buffer-size",    required_argument, 0, CH_TST_BUFFER_SIZE },
        {"help",           no_argument,       0, CH_TST_HELP },
        {NULL,             0,                 0, 0 }
    };
    for(;;) {
        c = getopt_long(
            argc,
            argv,
            "abc:d:012",
            long_options,
            &option_index
        );
        if(c == -1)
            break;
        switch(c) {
            case CH_TST_HELP:
                print_help(long_options);
                exit(1);
                break;
            case CH_TST_ALWAYS_ENCRYPT:
                printf("Set always encrypt\n");
                ch_chirp_set_always_encrypt();
                break;
            case CH_TST_MESSAGE_COUNT:
                _ch_tst_message_count = strtol(optarg, NULL, 10);
                if(errno) {
                    fprintf(stderr, "message-count must be integer.\n");
                    exit(1);
                }
                break;
            case CH_TST_BUFFER_SIZE:
                _ch_tst_buffer_size = strtol(optarg, NULL, 10);
                if(errno) {
                    fprintf(stderr, "buffer-size must be integer.\n");
                    exit(1);
                }
                break;
            case CH_TST_TIMEOUT:
                _ch_tst_timeout = strtof(optarg, NULL);
                if(errno) {
                    fprintf(stderr, "timeout must be float.\n");
                    exit(1);
                }
                break;
            default:
                fprintf(stderr, "unknown option\n");
                exit(1);
        }
    }
    ch_tst_chirp_thread_t sender_thread;
    ch_tst_chirp_thread_t echo_thread;
    sender_thread.other = &echo_thread;
    echo_thread.other = &sender_thread;
    sender_thread.port = PORT_SENDER;
    sender_thread.init = _ch_tst_sender_init_handler;
    echo_thread.port   = PORT_ECHO;
    echo_thread.init   = _ch_tst_echo_init_handler;
    uv_thread_t echo;
    uv_thread_t sender;
    ch_libchirp_init();
    uv_thread_create(&echo, _ch_tst_run_chirp, &echo_thread);
    sleep(1);
    uv_thread_create(&sender, _ch_tst_run_chirp, &sender_thread);
    uv_thread_join(&echo);
    uv_thread_join(&sender);
    ch_libchirp_cleanup();
    ch_qc_free_mem();
}
