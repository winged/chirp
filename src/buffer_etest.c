// ===================
// Testing buffer pool
// ===================
//
// Test runner if the buffer pool handles errors and misuse. Driven by
// hypothesis.
//
// Project includes
// ================
//
// .. code-block:: cpp
//
#include "libchirp.h"
#include "buffer.h"
#include "test_test.h"

typedef enum {
    func_init_e = 1,
    func_acquire_e = 2,
    func_release_e = 3,
    func_cleanup_e = 4,
} ch_tst_func_t;

static ch_buffer_pool_t pool;

// Runner
// ======
//
// .. code-block:: cpp
//
static
void
ch_tst_test_handler(mpack_node_t data, mpack_writer_t* writer)
{
    int func = mpack_node_int(
        mpack_node_array_at(data, 0)
    );
    int val = mpack_node_int(
        mpack_node_array_at(data, 1)
    );
    switch(func) {
        int ret;
        ch_bf_handler_t* handler;
        case func_init_e:
            ch_bf_init(&pool, val);
            ch_tst_return_int(writer, 0);
            break;
        case func_acquire_e:
            handler = ch_bf_acquire(&pool, &ret);
            mpack_start_array(writer, 2);
            mpack_write_int(writer, ret);
            if(handler == NULL)
                mpack_write_int(writer, -1);
            else
                mpack_write_int(writer, handler->id);
            mpack_finish_array(writer);
            break;
        case func_release_e:
            ch_bf_release(&pool, val);
            ch_tst_return_int(writer, 0);
            break;
        case func_cleanup_e:
            ch_bf_free(&pool);
            ch_tst_return_int(writer, 0);
            break;
        default:
            assert(0 && "Not implemented");
    }
}

int
main(void)
{
    ch_libchirp_init();
    int ret = mpp_runner(ch_tst_test_handler);
    ch_libchirp_cleanup();
    return ret;
}
