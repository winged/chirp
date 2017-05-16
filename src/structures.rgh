// =================
// Structures Header
// =================
//
// Functions for common data structures
//
// .. code-block:: cpp
//
#ifndef ch_structures_h
#define ch_structures_h

// Project includes
// ================
//
// .. code-block:: cpp
//
#include "common.h"

// System includes
// ===============
//
// .. code-block:: cpp
//

// Traits
// ======
//
// TODO: Document
//
// .. code-block:: cpp
//
#define CH_TR_MQEND(x) (x)->_qend
#define CH_TR_QEND(x) (x)->qend
#define CH_TR_MQNEXT(x) (x)->_next
#define CH_TR_QNEXT(x) (x)->next

#begindef CH_GQ_ITEM_INIT(item, next, end)
    next(item) = NULL;
    end(item) = NULL;
#enddef

#begindef CH_MQ_ITEM_INIT(item)
    CH_GQ_ITEM_INIT(item, CH_TR_MQNEXT, CH_TR_MQEND)
#enddef

#begindef CH_QQ_ITEM_INIT(item)
    CH_GQ_ITEM_INIT(item, CH_TR_QNEXT, CH_TR_QEND)
#enddef

#begindef CH_GQ_ENQUEUE(queue, item, next, end)
{
    A(next(item) == NULL, "Item already in use");
    A(end(item) == NULL, "Item already in use");
    if(queue == NULL) {
        /* The new item becomes the queue */
        queue = item;
    } else {
        /* Append the new item at the end of the queue */
        next(end(queue)) = item;
        /* Clear existing end pointers */
        end(end(queue)) = NULL;
    }
    /* The new item is the new end of the queue */
    end(queue) = item;
}
#enddef

#begindef CH_MQ_ENQUEUE(queue, item)
    CH_GQ_ENQUEUE(queue, item, CH_TR_MQNEXT, CH_TR_MQEND)
#enddef

#begindef CH_QQ_ENQUEUE(queue, item)
    CH_GQ_ENQUEUE(queue, item, CH_TR_QNEXT, CH_TR_QEND)
#enddef

#begindef CH_GQ_DEQUEUE(queue, next, end)
{
    if(queue != NULL) {
        void* __ch_gq_next_ = next(queue);
        if(__ch_gq_next_ != NULL) {
            /* Store the end in the new head */
            end(next(queue)) = end(queue);
        }
        /* Clear the old head */
        CH_GQ_ITEM_INIT(queue, next, end);
        /* next is the new head */
        queue = __ch_gq_next_;
    }
}
#enddef

#begindef CH_MQ_DEQUEUE(queue)
    CH_GQ_DEQUEUE(queue, CH_TR_MQNEXT, CH_TR_MQEND)
#enddef

#begindef CH_QQ_DEQUEUE(queue)
    CH_GQ_DEQUEUE(queue, CH_TR_QNEXT, CH_TR_QEND)
#enddef

#endif // ch_structures_h
