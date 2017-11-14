========
libchirp
========

Message-passing for everyone
============================

Features
========

* Fully automatic connection setup

* TLS support

  * Connections to 127.0.0.1 and ::1 aren't encrypted
  * We support and test with OpenSSL, but we prefer LibreSSL

* Easy message routing

* Robust

  * No message can be lost without an error (or it is a bug)

* Very thin API

* Minimal code-base, all additional features will be implemented as modules in
  an upper layer

* Fast

  * Up to 50'000 msg/s on a single-connection (encrypted 35'000 msg/s)
  * Up to 100'000 msg/s in star-topology (encrypted same)

    * Which shows that chirp is highly optimized, but still if the network delay
      is bigger star-topology is the way to go.

.. _modes-of-operation:

Modes of operation
==================

Connection-synchronous (config.ACKNOWLEDGE=1)
---------------------------------------------

* The send callback only returns a success when the remote has called
  ch_chirp_release_message

* No message can be lost by chirp

* For concurrency the application needs to copy the message and call
  ch_chirp_release message. Then the application has to take care that the
  copied message is not lost

* If the application call ch_chirp_release_message after the operation is
  finished, messages will automatically be throttled. Be aware of the timeout:
  if the applications operation takes longer either increase the timeout or copy
  the message (where by you lose the throttling)

* Slower

Connection-asynchronous (config.ACKNOWLEDGE=0)
----------------------------------------------

* The send callback returns a success when the message is successfully written to
  the operating system

* Message can be lost by chirp

* Automatic concurrency, by default chirp uses 16 concurrent handlers 

* The application needs a scheduler that periodically checks that operations
  have completed

* Faster

What should I use?
------------------

For simple message transmission, for example sending events to a time-series
database we recommend config.ACKNOWLEDGE=1, since chirp will cover this process
out of the box.

For more complex application where you have to schedule your operations anyway,
use config.ACKNOWLEDGE=0, do periodic bookkeeping and resend failed
operations.

Diving in
=========

.. toctree::
   :maxdepth: 2

   building.rst
   example.rst
   tutorial/index.rst

API Reference
=============

.. toctree::
   :maxdepth: 3

   include/libchirp/chirp.h.rst
   include/libchirp/message.h.rst
   include/libchirp/callbacks.h.rst
   include/libchirp/const.h.rst
   include/libchirp.h.rst
   include/libchirp/encryption.h.rst
   include/libchirp/error.h.rst
   include/libchirp/wrappers.h.rst
   include/libchirp/common.h.rst

.. note::

   The API is not thread-safe except where stated: Functions have \*_ts suffix.
   uv_async_send() can be used.

   Only one thread per :c:type:`ch_chirp_t` object is possible, the
   :c:type:`uv_loop_t` has to run in that thread. \*_ts functions can be called
   from any thread.


Additional information
======================

.. toctree::
   :maxdepth: 3

   development.rst


* :ref:`genindex`
* :ref:`search`
