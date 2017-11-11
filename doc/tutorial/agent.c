// =====
// Agent
// =====
//
// A agent
//
// Project includes
// ================
//
// .. code-block:: cpp
//
#include "libchirp.h"

// System includes
// ===============
//
// .. code-block:: cpp
//

// Declarations
// ============
//
// .. code-block:: cpp

static ch_chirp_t* _ch_tst_chirp;

// Definitions
// ============
//
// .. code-block:: cpp
//
//
static
void
_ch_tst_start(ch_chirp_t* chirp)
{
    (void)(chirp);
}

static
void
_ch_tst_sent_cb(
        ch_chirp_t* chirp,
        ch_message_t* msg,
        int status,
        float load
)
{
    (void)(chirp);
    (void)(status);
    (void)(load);
    ch_chirp_release_message(msg);
}
static
void
_ch_tst_recv_message_cb(ch_chirp_t* chirp, ch_message_t* msg)
{
    ch_chirp_send(
            chirp,
            msg,
            _ch_tst_sent_cb
    );
}

int
main(int argc, char *argv[])
{
    if(argc < 2) {
        fprintf(stderr, "%s listen_port\n", argv[0]);
        exit(1);
    }
    int port = strtol(argv[1], NULL, 10);
    if(errno) {
        fprintf(stderr, "port must be integer.\n");
        exit(1);
    }
    if(port <= 1024) {
        fprintf(stderr, "port must be greater than 1024.\n");
        exit(1);
    }
    if(port > 0xFFFF) {
        fprintf(stderr, "port must be lesser than %d.\n", 0xFFFF);
        exit(1);
    }
    ch_libchirp_init();
    ch_config_t config;
    ch_chirp_config_init(&config);
    config.PORT               = port;
    config.DISABLE_ENCRYPTION = 1;
    ch_chirp_run(
        &config,
        &_ch_tst_chirp,
        _ch_tst_recv_message_cb,
        _ch_tst_start,
        NULL,
        NULL
    );
    ch_libchirp_cleanup();
}
