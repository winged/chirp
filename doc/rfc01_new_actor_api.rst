======================
RFC 01 - New Actor API
======================

.. note::

   Since Python (CPython 2, 3 AND PyPy) is the primary target for C4irp we
   define the this API for Python. I'm pretty sure I can create some C-Macros to
   make the API as secure as the Python version, but I don't invest any time
   into this matter at the moment. If the C API isn't as secure as the Python
   version, its no big loss, since C is generally not as secure as Python.

Elements
========

Actor
   A function which receives messages. To be independent from different
   languages (that have no coroutines) they will be called from a thread-pool.

Node
   A process that registers a set of actors with the cluster.

Cluster
   Handles nodes/actors. It made up from the following elements: Registry,
   Watcher, Router.

Registry
   Raft based state-machine which keeps track of nodes/actors.

Watcher
   Checks if nodes are alive.

Router
   Sends messages to the node with lowest load.

Setups
======

Code written in one setup, will work in any other setup.

Single node
-----------

In the single node setup only one node and its actors exist. There is no need
for the cluster. Messages go directly to the actor (no network stack).

Local nodes
-----------

Nodes exist only on the localhost. Which means the registry isn't a raft
cluster, but everything else is active.

Full network
------------

Needs the complete stack.

Level one: No state at all
==========================

Our goal is to encourage the purest form of the actor model:

* Actors have no state
* Actors can send messages to other actors
* Actors can't wait for answers


.. code-block:: python

   def actor_one(message):
      res = do_some_work(message)
      for addr, port, actor, data in res:
         msg = chirp.Message(
            addr,
            port,
            actor,
            data
         )
         chirp.send(msg)

This means:

* You can scatter (map)
* But you can't gather (reduce)

Which makes the model incomplete.

It also means, if you have a large data-set:

* You have to either pass it along (maybe over the network)
* Or you have to read it from a database/disk on every message

Which is a nuisance, but does not make the model incomplete.

Level two: actor local state
============================

In order to gather results we need an actor with a local state. So while the
computing actors are still of the purest form: no state. We add an actor to
collect results.

We also introduce RoutedMessage which will send the message to the next
available node.

.. code-block:: python

   def actor_scatter(message):
      jobs = create_batch(message)
      for data in jobs:
         msg = chirp.RoutedMessage(
            "actor_work",
            data  # Contains a batch_id
         )
         chirp.send(msg)


   def actor_work(message):
      # Not interested in the batch_id
      res = do_work(message)
      msg = chirp.RoutedMessage(
         "actor_gather",
         data
      )
      chirp.send(msg)


   def actor_gather(message, local_state):
      with local_state as st:  # Locks the local state
         # Uses batch_id to gather the results
         add_to_state(message, st)
         if is_batch_complete(st):
            res = do_work(message)
            msg = chirp.RoutedMessage(
               "actor_next",
               next_from_batch(st)
            )
            chirp.send(msg)


The local state also allows to cache larger amounts of data, which improves
performance.

We introduce BoundMessage: It will first be routed to the next available actor
(if node is None). After that it will be sent to the same node. If the node is
unavailable it will be sent to the next available node.

.. code-block:: python

   def actor_alert_plan(message):
      alerts = create_alerts(message)
      node = None
      for alert, timeout in alerts:
         msg = chirp.BoundMessage(
            node,
            "actor_alert_user",
            alert
         )
         node = chirp.send(msg)
         time.sleep(timeout)
      msg = chirp.BoundMessage(
         node,
         "actor_alert_user",
         clear_message(message)
      )


   def actor_alert_user(message, local_state):
      alering_info = None
      if is_clear_message(message):
         clear_alerting_info(message, local_state)
      else:
         with local_state as st:  # Locks the local state
            if not has_alert_info(message, st):
               load_alert_info_from_db(message, st)
               chirp.schedule(
                  get_max_plan_time(message)  # the timeout
                  clear_alerting_info,
                  local_state,
               )  # We have ALWAYS to clear the cache in case of network
                  # or node failures
            alerting_info = get_alerting_info(message, st)
         alert_user(alerting_info)

If we want the alert plan to be stoppable we need a local state for the alert
plan, too.

.. code-block:: python

   def actor_alert_plan(message, local_state):
      if is_cancel_message(message):
         with local_state as st:
            cancel_alert_plan(message, st)
            msg = chirp.BoundMessage(
               node_from_plan(message, st),
               "actor_alert_user",
               clear_message(message)
            )
      else:
         alerts = create_alerts(message)
         active = False
         node = None
         with local_state as st:
            active = is_active(message, st)
         while active:
            alert, timeout = next(alerts)
            msg = chirp.BoundMessage(
               node, 
               "actor_alert_user",
               alert
            )
            node = chirp.send(msg)
            with local_state as st:
               store_node_for_plan(st, node)
            time.sleep(timeout)
            with local_state as st:
               active = is_active(message, st)
         msg = chirp.BoundMessage(
            node,
            "actor_alert_user",
            clear_message(message)
         )

It looks like this is a lot code-overhead, but the above example is adapted
from code we use in production implemented in tornado. And since you need to
lock data-structures with coroutines too. The above cleaner than the tornado
based code.


Level three: actor global state
===============================

The actor model is now complete. One can always use C4irp as messaging layer for
RPC, but of course with the given API it is up to the user to implement that.
With another simple and clean extension to the model. RPC will be possible.

.. code-block:: python

   def actor_doing_rcp(message, local_state, global_state):
      with global_state as st:
         rpc = need_users(st)
      if rpc:
         msg = chirp.Message(
            addr,
            port,
            "actor_get_users",
            data
         )
         chirp.send(msg)
      
   def actor_get_users(message):
      users = get_users(message)
      message.actor = "actor_get_users_res"
      message.data = users
      chirp.send(message)

   def actor_get_users_res(message, local_state, global_state):
      with global_state as st:
         set_users(message, st)

I used this model (without the global_state API though) implementing Raft and it
is completely straight forward. The only thing that will be needed is way to
access the global_state from scheduled callbacks.

.. code-block:: python

   def on_clear_users():
      with chirp.global_state as st:
         clear_users(st)

Deadlocks
---------

To prevent deadlocks this will not be possible:

.. code-block:: python

   def actor_one(message, local_state, global_state):
      with global_state as st:
         with local_state as st:
            pass

   def actor_two(message, local_state, global_state):
      with local_state as st:
         with global_state as st:
            pass

Only either the global or the local state may be locked by the same actor.

TODO: Can we actually implement that?? Since the global state may locked by
another actor which is ok.

But we need fine-grained locking
--------------------------------

No!

The model is 100% deadlock free, since we don't allow locking the global and the
local state at the same time in the same actor. As soon as we allow finer
grained locking deadlocks become possible.

Keep in mind that every node has its own global_state so the bottle-neck is only
per node. And if keep the lock time short there is no problem at all. And we
encourage the user not use the global_state anyways!

.. code-block:: python

   def actor_one(message, local_state, global_state):
      with global_state as st:
         work = checkout_work(st)
      do_a_lot_of_work(work)
      with global_state as st:
         commit_work(work, st)

Unittesting
===========

C-API
=====
