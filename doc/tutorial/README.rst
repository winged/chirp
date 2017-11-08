========
Tutorial
========

How to build
============

Copy:

   * libchirp.h
   * libchirp.c
   * libchirp-config.h

into this directory.

.. code-block:: bash

   make

How to get libchirp*
--------------------

* Either download a release of libchirp

* Or see "How to create a source distribution" in the README.rst of libchirp

  * If you want debug output do the following in build of libchirp

.. code-block:: bash

   make clean dist NO_UNIFDEF=True
