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

// Queues
// ======
//
// The queue pointer will always point to the first element in the queue.
//
// Enqueue operations will append the new element at the end of the queue (3).
// The qend pointer will be updated to the new end.
//
// Dequeue operations will update the queue pointer to the next element (2).
// The pointers (qend/next) on the current element (1) will be cleared. The
// qend pointer on (2) will be set.
//
// Any operations will clear unused pointer and enqueue will assert, that there
// are no pointers set (which probably means that the item is queued elsewhere).
//
// You can enqueue a item on a NULL-pointer: the pointer will be set. After you
// dequeue the last item, the pointer will be set to NULL.
//
// .. code-block:: text
//
//                      queue
//                        |
//                        v
//    .---.    .---.    .---.
//    | 3 |<---| 2 |<---| 1 |
//    '---'    '---'    '-.-'
//      ^                /
//       \              /
//        -----qend----'
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

// .. code-block:: cpp
//
#begindef CH_GQ_ENQUEUE(queue, item, next, end)
{
    A(next(item) == NULL, "Item already in use");
    A(end(item) == NULL, "Item already in use");
    if(queue == NULL) {
        /* The new item becomes the queue */
        queue = item;
    } else {
        A(next(end(queue)) == NULL, "End item used in different queue");
        /* Append the new item at the end of the queue */
        next(end(queue)) = item;
        /* Clear existing end pointers */
        end(end(queue)) = NULL;
    }
    /* The new item is the new end of the queue */
    end(queue) = item;
}
#enddef

// .. c:function:: CH_MQ_ENQUEUE(queue, item)
//
//    Enqueue a message onto a message queue. The item will become the queue's
//    head.
//
//    :param queue: Pointer to the queue
//    :param item: Pointer to the item
//
// .. code-block:: cpp
//
#begindef CH_MQ_ENQUEUE(queue, item)
    CH_GQ_ENQUEUE(queue, item, CH_TR_MQNEXT, CH_TR_MQEND)
#enddef

// .. c:function:: CH_QQ_ENQUEUE(queue, item)
//
//    Enqueue a item onto queue. The item will become the queue's head.
//
//    :param queue: Pointer to the queue
//    :param item: Pointer to the item
//
// .. code-block:: cpp
//
#begindef CH_QQ_ENQUEUE(queue, item)
    CH_GQ_ENQUEUE(queue, item, CH_TR_QNEXT, CH_TR_QEND)
#enddef

// .. code-block:: cpp
//
#begindef CH_GQ_DEQUEUE(queue, next, end)
{
    if(queue != NULL) {
        A(next(end(queue)) == NULL, "End item used in different queue");
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

// .. c:function:: CH_MQ_DEQUEUE(queue)
//
//    Dequeue a message. The queue points to the current item and it will point
//    to the next item after dequeue. If the queue is empty, queue will become
//    NULL.
//
//    :param queue: Pointer to the queue
//
// .. code-block:: cpp
//
#begindef CH_MQ_DEQUEUE(queue)
    CH_GQ_DEQUEUE(queue, CH_TR_MQNEXT, CH_TR_MQEND)
#enddef

// .. c:function:: CH_MQ_DEQUEUE(queue)
//
//    Dequeue a item. The queue points to the current item and it will point to
//    the next item after dequeue. If the queue is empty, queue will become
//    NULL.
//
//    :param queue: Pointer to the queue
//
// .. code-block:: cpp
//
#begindef CH_QQ_DEQUEUE(queue)
    CH_GQ_DEQUEUE(queue, CH_TR_QNEXT, CH_TR_QEND)
#enddef

#endif // ch_structures_h
