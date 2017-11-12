========
libchirp
========

Message-passing for everyone

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


Example echo-server
===================

First includes and declarations: We need a callback that is
called when a message is received and a callback that is called when the
echo-message is sent.

.. code-block:: cpp

   #include <libchirp.h>

   static
   void
   recv_message_cb(ch_chirp_t* chirp, ch_message_t* msg);

   static
   void
   sent_cb(ch_chirp_t* chirp, ch_message_t* msg, int status);

   int
   main(int argc, char *argv[])
   {

Next we have to initialize libchirp using :c:func:`ch_libchirp_init`. This will
initialize the TLS library (LibreSSL or OpenSSL) and a mutex used during
initialization of chirp-obejcts.

.. code-block:: cpp

       ch_libchirp_init();

:c:func:`ch_chirp_config_init` will initialize the config to its default values.

.. code-block:: cpp

       ch_config_t config;
       ch_chirp_config_init(&config);

Next we have to set a certification chain. You can create it using the makepki
Makefile_ on github. If you want to create it manually the chain has to contain:

* The certification authority's public key

* The client public key (signed by CA)

* The client private key

Any client-key signed by the CA will be able to connect.

.. _Makefile: https://github.com/concretecloud/chirp/tree/master/mk/makepki
       
.. code-block:: cpp

       config.CERT_CHAIN_PEM = "./cert.pem";
       config.DH_PARAMS_PEM  = "./dh.pem";

:c:func:`ch_chirp_run` initializes libuv, chirp, and runs main-loop.

.. code-block:: cpp

       ch_chirp_t chirp;
       ch_chirp_run(
           &config,
           &chirp,
           recv_message_cb,
           NULL,
           NULL,
           NULL
       );

If the user hits ctrl-c, the main-loop will close and we cleanup libchirp using
:c:func:`ch_libchirp_cleanup`.

.. code-block:: cpp

       ch_libchirp_cleanup();
   }

:c:func:`ch_chirp_send` will send the received message and then call send_cb.

.. code-block:: cpp

   static
   void
   recv_message_cb(ch_chirp_t* chirp, ch_message_t* msg)
   {
       ch_chirp_send(chirp, msg, sent_cb);
   }

The memory reserved for the message has to be retained till the message is sent
or has failed, so we release the receive handler after message is successfully
send. :c:func:`ch_chirp_release_message`

.. code-block:: cpp

   static
   void
   sent_cb(ch_chirp_t* chirp, ch_message_t* msg, int status)
   {
       ch_chirp_release_message(msg);
   }


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
