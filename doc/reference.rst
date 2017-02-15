=============
API Reference
=============

Public C API
============

.. note::

   The C API is not thread-safe except where stated: Functions have \*_ts suffix.
   uv_async_send() can be used.

.. toctree::
   :maxdepth: 2

   inc/libchirp.h.rst
   inc/libchirp/callbacks.h.rst
   inc/libchirp/chirp.h.rst
   inc/libchirp/const.h.rst
   inc/libchirp/common.h.rst
   inc/libchirp/encryption.h.rst
   inc/libchirp/error.h.rst
   inc/libchirp/message.h.rst
   inc/libchirp/wrappers.h.rst

Internal C API
==============

.. toctree::
   :maxdepth: 2
   :includehidden:

   src/buffer.h.rst
   src/chirp.h.rst
   src/chirp.c.rst
   src/connection.h.rst
   src/connection.c.rst
   src/encryption.h.rst
   src/encryption.c.rst
   src/message.h.rst
   src/message.c.rst
   src/protocol.h.rst
   src/protocol.c.rst
   src/reader.h.rst
   src/reader.c.rst
   src/util.h.rst
   src/util.c.rst
   src/writer.h.rst
   src/writer.c.rst

Tests in C
==========

.. toctree::
   :maxdepth: 2
   :includehidden:

   src/connection_test.h.rst
   src/connection_test.c.rst
   src/pool_test.h.rst
   src/pool_test.c.rst

Test binaries
=============

.. toctree::
   :maxdepth: 2
   :includehidden:

   src/chirp_etest.c.rst

External Libs
=============

.. toctree::
   :maxdepth: 2
   :includehidden:

   src/sglib.h.rst
