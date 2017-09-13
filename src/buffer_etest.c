// ===================
// Testing buffer pool
// ===================
//
// Test if the buffer pool handles errors and misuse.
//
// Project includes
// ================
//
// .. code-block:: cpp
//
#include "quickcheck_test.h"
#include "buffer.h"

// Definitions
// ===========
//
// .. code-block:: cpp
//
typedef struct ch_safe_max_s {
    double a;
    void* b;
} ch_safe_max_t;

struct ch_tst_buffer_s;
typedef struct ch_tst_buffer_s ch_tst_buffer_t;
struct ch_tst_buffer_s {
    ch_bf_handler_t* handler;
    ch_tst_buffer_t*     left;
    ch_tst_buffer_t*     right;
    ch_tst_buffer_t*     parent;
    char             color;
};

#define ch_tst_bf_cmp_m rb_pointer_cmp_m
rb_bind_m(ch_tst_bf, ch_tst_buffer_t)

// Test functions
// ==============
//
// .. code-block:: cpp

static
void
ch_tst_gen_plan(ch_buf* data)
{
    int override = ch_qc_tgen_bool();
    int override_with = ch_qc_tgen_bool();
    int override_len = ch_qc_tgen_double();
    ch_qc_mem_track_t* mem = ch_qc_tgen_array(ch_qc_gen_bool, int);
    /* A plan is a list of acquire and release actions on the buffer pool*/
    int* plan = (int*) mem->data;
    int override_index = mem->count * override_len;
    /* Full pools are not very probable, so we override the plan to make ensure
     * that the pool is sometimes empty/full. */
    if(override)
        for(int i = override_index; i < mem->count; i++)
            plan[i] = override_with;
    ch_qc_return(ch_qc_mem_track_t*, mem);
}

void
ch_tst_print_plan(ch_buf* data)
{
    ch_qc_mem_track_t* mem = ch_qc_args(
        ch_qc_mem_track_t*,
        0,
        ch_qc_mem_track_t*
    );
    int* plan = (int*) mem->data;

    for(int i = 0; i < mem->count; i++)
        printf("%d ", plan[i]);
}

static
int
ch_tst_plan_works(ch_buf* data)
{
    int ret = 1;
    ch_tst_buffer_t* buffers = NULL;
    ch_tst_bf_tree_init(&buffers);
    ch_buffer_pool_t pool;
    ch_qc_mem_track_t* mem = ch_qc_args(
        ch_qc_mem_track_t*,
        0,
        ch_safe_max_t
    );
    double fsize = ch_qc_args(
        double,
        1,
        ch_safe_max_t
    );
    int size = fsize * 40;
    if(size > 32)
        size = 32;
    ch_bf_init(&pool, size);
    int* plan = (int*) mem->data;
    int count = 0;
    for(int i = 0; i < mem->count; i++) {
        int action = plan[i];
        /* We execute acquire/release according to the plan */
        if(action) {
            int last = 0;
            ch_bf_handler_t* handler = ch_bf_acquire(&pool, &last);
            if(count < size) {
                if(handler == NULL) {
                    printf("No handler, before size was reached.\n");
                    ret = 0;
                    break;
                }
            } else {
                if(handler != NULL) {
                    printf("Got handler, after size was reached.\n");
                    ret = 0;
                    break;
                }
            }
            if(handler != NULL) {
                ch_tst_buffer_t* buffer = ch_alloc(sizeof(ch_tst_buffer_t));
                ch_tst_bf_node_init(buffer);
                buffer->handler = handler;
                ch_tst_bf_insert(&buffers, buffer);
                count += 1;
                if(last) {
                    if(count != size) {
                        printf("Last flag not set correctly.\n");
                        ret = 0;
                        break;
                    }
                    if(ch_bf_acquire(&pool, &last) != NULL) {
                        printf("Got buffer after last.\n");
                        ret = 0;
                        break;
                    }
                }
            }
        } else {
            if(count > 0) {
                ch_tst_buffer_t* buffer = buffers;
                ch_bf_release(&pool, buffer->handler->id);
                ch_tst_bf_delete_node(&buffers, buffer);
                ch_free(buffer);
                count -= 1;
            }
        }
    }
    while(buffers != ch_tst_bf_nil_ptr) {
        ch_tst_buffer_t* buffer = buffers;
        ch_bf_release(&pool, buffer->handler->id);
        ch_tst_bf_delete_node(&buffers, buffer);
        ch_free(buffer);
    }
    ch_bf_free(&pool);
    return ret;
}

// Runner
// ======
//
// .. code-block:: cpp

int
main(void)
{
    ch_qc_init();
    ch_qc_gen gs0[] = { ch_tst_gen_plan, ch_qc_gen_double };
    ch_qc_print ps0[] = { ch_tst_print_plan, ch_qc_print_double };
    printf("Testing acquire/release plan: ");
    return !ch_qc_for_all(ch_tst_plan_works, 2, gs0, ps0, ch_safe_max_t);
}
