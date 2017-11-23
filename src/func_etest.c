// ========================
// Hypthesis testing driver
// ========================
//
// Executes the actions planned by hypothesis.
//
// Project includes
// ================
//
// .. code-block:: cpp
//
#include "common.h"
#include "libchirp.h"
#include "rbtree.h"
#include "test_test.h"

typedef enum {
    func_42_e             = 1,
    func_cleanup_e        = 2,
    func_send_message_e   = 3,
    func_check_messages_e = 4,
} ch_tst_func_t;

struct ch_tst_msg_tree_s;
typedef struct ch_tst_msg_tree_s ch_tst_msg_tree_t;
struct ch_tst_msg_tree_s {
    ch_message_t       msg;
    int32_t            status;
    char               color;
    ch_tst_msg_tree_t* parent;
    ch_tst_msg_tree_t* left;
    ch_tst_msg_tree_t* right;
};

#define ch_tst_identity_m(x) (x)->msg.identity

#define _ch_tst_msg_cmp_m(x, y)                                                \
    memcmp(ch_tst_identity_m(x), ch_tst_identity_m(y), CH_ID_SIZE)

rb_bind_m(_ch_tst_msg, ch_tst_msg_tree_t)

        static int _ch_tst_always_encrypt = 0;
static ch_chirp_t*        _ch_tst_chirp;
static mpack_node_t       _ch_tst_cur_mpack_data;
static mpack_writer_t*    _ch_tst_cur_mpack_writer;
static uv_sem_t           _ch_tst_sem;
static uv_thread_t        _ch_tst_runner;
static uv_async_t         _ch_tst_async_handler;
static ch_tst_msg_tree_t* _ch_tst_msg_tree;
static uv_timer_t         _ch_tst_wait_msgs;
static int                _ch_tst_wait_count = 0;

static void
_ch_tst_check_messages(mpack_writer_t* writer);

// Runner
// ======
//
// .. code-block:: cpp
//
static void
_ch_tst_next()
{
    uv_sem_post(&_ch_tst_sem);
}

static void
_ch_tst_close_cb2(uv_handle_t* handle)
{
    (void) (handle);
    ch_chirp_close_ts(_ch_tst_chirp);
}

static void
_ch_tst_close_cb1(uv_handle_t* handle)
{
    (void) (handle);
    uv_timer_stop(&_ch_tst_wait_msgs);
    uv_close((uv_handle_t*) &_ch_tst_wait_msgs, _ch_tst_close_cb2);
}

static void
_ch_tst_send_cb(ch_chirp_t* chirp, ch_message_t* msg, int status)
{
    (void) (chirp);
    ch_tst_msg_tree_t* entry = msg->user_data;
    entry->status            = status;
}

static void
_ch_tst_send_message(mpack_writer_t* writer, ch_ip_protocol_t proto, int port)
{
    ch_tst_msg_tree_t* entry = ch_alloc(sizeof(*entry));
    entry->status            = 0x0ddba11; /* no status */
    _ch_tst_msg_node_init(entry);
    ch_message_t* msg = &entry->msg;
    ch_msg_init(msg);
    msg->user_data = entry;
    int ret        = _ch_tst_msg_insert(&_ch_tst_msg_tree, entry);
    if (proto == AF_INET6) {
        ch_msg_set_address(msg, proto, "::1", port);
    } else {
        ch_msg_set_address(msg, proto, "127.0.0.1", port);
    }
    mpack_start_array(writer, 2);
    mpack_write_bin(writer, (char*) msg->identity, CH_ID_SIZE);
    mpack_write_int(writer, ret);
    mpack_finish_array(writer);
    ch_chirp_send(_ch_tst_chirp, msg, _ch_tst_send_cb);
}

static void
_ch_tst_recheck_messages(uv_timer_t* handle)
{
    mpack_writer_t* writer = handle->data;
    if (_ch_tst_wait_count < 5) {
        _ch_tst_check_messages(writer);
    } else {
        mpack_start_array(writer, 0);
        mpack_finish_array(writer);
        _ch_tst_next();
    }
}

static void
_ch_tst_check_messages(mpack_writer_t* writer)
{
    _ch_tst_wait_count += 1;
    if (_ch_tst_msg_tree == NULL) {
        mpack_start_array(writer, 0);
        mpack_finish_array(writer);
        _ch_tst_next();
    } else {
        int done = 1;
        rb_iter_decl_cx_m(_ch_tst_msg, iter, elem);
        rb_for_m (_ch_tst_msg, _ch_tst_msg_tree, iter, elem) {
            if (elem->status == 0x0ddba11) {
                done = 0;
                break;
            }
        }
        if (done) {
            int size = _ch_tst_msg_size(_ch_tst_msg_tree);
            mpack_start_array(writer, size);
            while (_ch_tst_msg_tree != _ch_tst_msg_nil_ptr) {
                elem = _ch_tst_msg_tree;
                _ch_tst_msg_delete_node(&_ch_tst_msg_tree, elem);
                mpack_write_bin(writer, (char*) elem->msg.identity, CH_ID_SIZE);
                ch_free(elem);
            }
            mpack_finish_array(writer);
            _ch_tst_next();
        } else {
            _ch_tst_wait_msgs.data = writer;
            uv_timer_start(
                    &_ch_tst_wait_msgs, _ch_tst_recheck_messages, 100, 0);
        }
    }
}

static void
_ch_tst_async_mpack_handler_cb(uv_async_t* handle)
{
    L(_ch_tst_chirp, "Mpack call", CH_NO_ARG);
    (void) (handle);
    mpack_writer_t* writer = _ch_tst_cur_mpack_writer;
    mpack_node_t    data   = _ch_tst_cur_mpack_data;

    int func = mpack_node_int(mpack_node_array_at(data, 0));
    switch (func) {
    case func_42_e: {
        ch_tst_return_int(writer, 42);
        _ch_tst_next();
        break;
    }
    case func_cleanup_e: {
        uv_close((uv_handle_t*) handle, _ch_tst_close_cb1);
        break;
    }
    case func_send_message_e: {
        int proto = mpack_node_int(mpack_node_array_at(data, 1));
        int port  = mpack_node_int(mpack_node_array_at(data, 2));
        _ch_tst_send_message(writer, proto, port);
        _ch_tst_next();
        break;
    }
    case func_check_messages_e: {
        _ch_tst_wait_count = 0;
        _ch_tst_check_messages(writer);
        break;
    }
    default:
        assert(0 && "Not implemented");
    }
}

static void
_ch_tst_mpack_handler_cb(mpack_node_t data, mpack_writer_t* writer)
{
    _ch_tst_cur_mpack_writer = writer;
    _ch_tst_cur_mpack_data   = data;
    uv_async_send(&_ch_tst_async_handler);
    uv_sem_wait(&_ch_tst_sem);
}

static void
_ch_tst_run_runner(void* arg)
{
    (void) (arg);
    assert(mpp_runner(_ch_tst_mpack_handler_cb) == CH_SUCCESS);
}

static void
_ch_tst_start_cb(ch_chirp_t* chirp)
{
    ch_chirp_check_m(chirp);
    L(chirp, "Chirp started", CH_NO_ARG);
    uv_loop_t* loop = ch_chirp_get_loop(chirp);
    uv_async_init(loop, &_ch_tst_async_handler, _ch_tst_async_mpack_handler_cb);
    uv_timer_init(loop, &_ch_tst_wait_msgs);
    if (_ch_tst_always_encrypt) {
        ch_chirp_set_always_encrypt(chirp);
    }
    uv_thread_create(&_ch_tst_runner, _ch_tst_run_runner, chirp);
}

static void
_ch_tst_done_cb(ch_chirp_t* chirp)
{
    (void) (chirp);
    ch_chirp_check_m(chirp);
    ch_tst_return_int(_ch_tst_cur_mpack_writer, 0);
    _ch_tst_next();
    L(chirp, "Chirp done", CH_NO_ARG);
}

static void
_ch_tst_recv_message_cb(ch_chirp_t* chirp, ch_message_t* msg)
{
    (void) (chirp);
    ch_chirp_release_message(msg);
}
int
main(int argc, char* argv[])
{
    ch_libchirp_init();
    uv_sem_init(&_ch_tst_sem, 0);

    if (argc < 3) {
        fprintf(stderr, "%s listen_port always_encrypt\n", argv[0]);
        exit(1);
    }
    int port = strtol(argv[1], NULL, 10);
    if (errno) {
        fprintf(stderr, "port must be integer.\n");
        exit(1);
    }
    if (port <= 1024) {
        fprintf(stderr, "port must be greater than 1024.\n");
        exit(1);
    }
    if (port > 0xFFFF) {
        fprintf(stderr, "port must be lesser than %d.\n", 0xFFFF);
        exit(1);
    }
    _ch_tst_always_encrypt = strtol(argv[2], NULL, 10);
    if (errno) {
        fprintf(stderr, "always_encrypt must be integer.\n");
        exit(1);
    }
    if (!(_ch_tst_always_encrypt == 0 || _ch_tst_always_encrypt == 1)) {
        fprintf(stderr, "always_encrypt must be boolean (0/1).\n");
        exit(1);
    }
    _ch_tst_msg_tree_init(&_ch_tst_msg_tree);
    ch_config_t config;
    ch_chirp_config_init(&config);
    config.PORT           = port;
    config.CERT_CHAIN_PEM = "./cert.pem";
    config.DH_PARAMS_PEM  = "./dh.pem";
    int ret               = ch_chirp_run(
            &config,
            &_ch_tst_chirp,
            _ch_tst_recv_message_cb,
            _ch_tst_start_cb,
            _ch_tst_done_cb,
            NULL);
    uv_thread_join(&_ch_tst_runner);
    uv_sem_destroy(&_ch_tst_sem);
    ch_libchirp_cleanup();
    return ret;
}