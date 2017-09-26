=====================
Concrete Clouds Chirp
=====================

API Reference
=============

.. note::

   The API is not thread-safe except where stated: Functions have \*_ts suffix.
   uv_async_send() can be used.

   Only one thread per :c:type:`ch_chirp_t` object is possible, the
   :c:type:`uv_loop_t` has to run in that thread. \*_ts functions can be called
   from any thread.

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

Additional information
======================

.. toctree::
   :maxdepth: 3

   development.rst


* :ref:`genindex`
* :ref:`search`
