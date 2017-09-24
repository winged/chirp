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

   inc/libchirp/chirp.h.rst
   inc/libchirp/message.h.rst
   inc/libchirp/callbacks.h.rst
   inc/libchirp/const.h.rst
   inc/libchirp.h.rst
   inc/libchirp/encryption.h.rst
   inc/libchirp/error.h.rst
   inc/libchirp/wrappers.h.rst
   inc/libchirp/common.h.rst

Additional information
======================

.. toctree::
   :maxdepth: 3

   development.rst


* :ref:`genindex`
* :ref:`search`
