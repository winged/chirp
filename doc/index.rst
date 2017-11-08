=====================
Concrete Clouds Chirp
=====================

Example
=======

First some boiler-plate (includes and declarations): We need a callback that is
called when a message is received and a callback that is called when the
echo-message is sent.

.. code-block:: bash

   #include <libchirp.h>

   static
   void
   recv_message_cb(ch_chirp_t* chirp, ch_message_t* msg);

   static
   void
   sent_cb(ch_chirp_t* chirp, ch_message_t* msg, int status, float load);

   int
   main(int argc, char *argv[])
   {

Next we have to initialize libchirp. This will initialize the TLS library
(LibreSSL or OpenSSL) and a mutex used during initialization of chirp-obejcts.

.. code-block:: bash

       ch_libchirp_init();

ch_chirp_config_init will initialize the config to its default values.

.. code-block:: bash

       ch_config_t config;
       ch_chirp_config_init(&config);

Next we have to set a certification chain. You can create it using the makepki
Makefile_ on github. If you want to create it manually the chain has to contain:

 * The certification authority's public key

 * The client public key (signed by CA)

 * The client private key

Any client-key signed by the CA will be able to connect.

.. _Makefile: https://github.com/concretecloud/chirp/tree/master/mk/makepki
       
.. code-block:: bash

       config.CERT_CHAIN_PEM = "./cert.pem";
       config.DH_PARAMS_PEM  = "./dh.pem";

ch_chirp_run is function that initializes libuv, chirp, and runs main-loop.

.. code-block:: bash

       ch_chirp_t chirp;
       ch_chirp_run(
           &config,
           &chirp,
           recv_message_cb,
           NULL,
           NULL,
           NULL
       );

If the user hits ctrl-c, the main-loop will close and we cleanup libchirp.

.. code-block:: bash

       ch_libchirp_cleanup();
   }

ch_chirp_send will send the received message and then call send_cb.

.. code-block:: bash

   static
   void
   recv_message_cb(ch_chirp_t* chirp, ch_message_t* msg)
   {
       ch_chirp_send(chirp, msg, sent_cb);
   }

The memory reserved for the message has to be retained till the message is sent
or has failed, so we release the receive handler after message is successfully
send.

.. code-block:: bash

   static
   void
   sent_cb(ch_chirp_t* chirp, ch_message_t* msg, int status, float load)
   {
       ch_chirp_release_recv_handler(msg);
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
