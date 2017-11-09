=============================
Building and installing Chirp
=============================

Unix
====

Build dependencies:

* python3 [3]_

* make

* gcc or clang

Documentation build dependencies:

* sphinx

* graphviz

Dependencies:

* libuv

* openssl


Install to prefix /usr/local. (with docs)

.. code-block:: bash

   cd build
   ../configure --doc
   make
   make check
   sudo make install

Install to prefix /usr. (without docs)

.. code-block:: bash

   cd build
   ../configure --prefix /usr
   make
   make check
   sudo make install

Install to prefix /usr, but copy to package dir. (Package creation)

.. code-block:: bash

   cd build
   ../configure --prefix /usr
   make
   make check
   make install DEST=pkgdir

.. _source_dist:

How to create a source distribution
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Building a source distribution is useful when you need to include libchirp in
your project, but don't want to use it's build system. A source distribution can
easily be compiled with just a ``make`` call.

.. code-block:: bash

   cd build
   ../configure --dest --doc
   make dist
   ls dist

.. [3] Script-headers can be patched to work with python2.

Windows
-------

We want to support Windows, but we are currently not building on Windows. VS
2013 or newer should support all C99 feature we use.



.. vim: set spell spelllang=en foldmethod=marker sw=2 ts=2 et tw=76: .. }}}

