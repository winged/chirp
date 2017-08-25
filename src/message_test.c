// ============
// Message test
// ============
//
// .. code-block:: cpp

// Project includes
// ================
//
// .. code-block:: cpp

#include "quickcheck_test.h"
#include "message.h"
#include "util.h"

// Definitions
// ===========
//

// .. c:function::
static
size_t _ch_test_gen_data_field(
        float zero_probability,
        float max_probability,
        int max_size,
        ch_buf** data
)
//
//   Allocates memory for one of the data field in a chirp message: header,
//   actor, data.
//
//    :param zero_probability: Probability of the len being zero
//    :param max_probability: Probability of the len being max
//    :param int max_size: Max size of that field
//    :param ch_buf*: Buffer for the data
//
// .. code-block:: cpp
//
{
    ch_qc_mem_track_t* track;
    double draw = ch_qc_tgen_double();
    size_t size = 0;
    if(draw > zero_probability) {
        if(draw < max_probability)
            size = max_size;
        else {
            size = ch_qc_tgen_double() * max_size;
        }
    }
    if(size == 0)
        return size;
    track = ch_qc_tgen_bytes();
    track->data = ch_realloc(track->data, size);
    if(track->size < size)
        memset(track->data + track->size, 0, size - track->size);
    track->size = 0;
    *data = track->data;
    return size;
}


// .. c:function::
ch_message_t*
ch_test_gen_message(struct ch_chirp_s* chirp)
//    :noindex:
//
//    see: :c:func:`ch_ms_gen_message`
//
// .. code-block:: cpp
//
{
    ch_message_t* message;
    int data_size = 1024;
    int big = ch_qc_tgen_bool();
    if(big) {
        int very = ch_qc_tgen_bool();
        if(very)
            data_size = 1024 * 1024;
        else
            data_size = 1024 * 256;
    }
    ch_qc_mem_track_t* track;
    track = ch_qc_track_alloc(sizeof(ch_message_t));
    message = (ch_message_t*) track->data;
    ch_msg_init(chirp, message);
    message->header_len = _ch_test_gen_data_field(
        0.1,
        0.1,
        1024,
        &message->header
    );
    message->actor_len = _ch_test_gen_data_field(
        0.1,
        0.1,
        2048,
        &message->actor
    );
    message->data_len = _ch_test_gen_data_field(
        0.1,
        0.05,
        data_size,
        &message->data
    );
    int ipv6 = ch_qc_tgen_bool();
    ipv6 = 0; // TODO remove
    if(ipv6)
        ch_msg_set_address(
            message,
            CH_IPV6,
            "::1",
            59732
        );
    else
        ch_msg_set_address(
            message,
            CH_IPV4,
            "127.0.0.1",
            59732
        );
    return message;
}
