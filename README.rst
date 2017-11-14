========
libchirp
========

Message-passing for everyone

|travis| |rtd|

.. |travis|  image:: https://travis-ci.org/concretecloud/chirp.svg?branch=master
   :target: https://travis-ci.org/concretecloud/chirp
.. |rtd| image:: https://1042.ch/ganwell/docs-master.svg
   :target: https://1042.ch/chirp/

`Read the Docs`_

.. _`Read the Docs`: http://1042.ch/chirp/

WORK IN PROGRESS
================

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

Install
=======

libchirp is distributed as an amalgamation (only needs make and compiler). See
`DIST-README.rst`_. The information below applies to the git repository.

.. _`DIST-README.rst`: https://github.com/concretecloud/chirp/blob/master/mk/DIST-README.rst

Unix
----

Dependencies:

* libuv

* libressl or openssl

Build dependencies:

* python3 [3]_

* make

* gcc or clang

Documentation build dependencies:

* sphinx

* graphviz

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


Development
===========

Test dependencies:

* cppcheck
* abi-compliance-checker

Unix
----

.. code-block:: bash

   cd build
   ../configure --dev
   make test

In development mode the make file has a help:

.. code-block:: bash

   make

Chirp has a mode to debug macros:

.. code-block:: bash

   ../configure --dev
   make clean all MACRO_DEBUG=True
   gdb src/message_etest

This requires clang-format to be installed.

Running pytest manually with -s for example:

.. code-block:: bash

   cd build
   make all
   pytest -s ../src

Or with gdb attached to the runner binary:

.. code-block:: bash

   MPP_GDB=True pytest -s ../src/

If you want to stop debugging, but pytest is restarting GDB endlessly:

.. code-block:: bash

   killall pytest

Docker
------

If a tool is not available on your platform or you have a old version of
cppcheck (cppcheck is known to behave very different across versions), you can
use the docker based tests.

.. code-block:: bash

   ./ci/alpine.sh

Travis will also run this script, so you can also use it to reproduce errors on
travis.

You can also run a shell.

.. code-block:: bash

   ./ci/alpine.sh shell

.. code-block:: bash

   ./ci/arch.sh shell

Note: Docker must have IPv6 enabled. Since we only need loopback, you can
configure a unique local subnet. For some reason docker doesn't support loopback
only anymore. I consider it a bug, the corresponding issue told me it isn't.

.. code-block:: bash

   DOCKER_OPTS="--ipv6 --fixed-cidr-v6 fc00:beef:beef::/40"

If IPv6 is working in your docker, you don't have to change anything. We only
need to loopback. The above is just how I solved the problem.

Windows
-------

No development build available.

Check vs test
-------------

make check
    Not instrumented (release mode), goal: checking compatibility

make test
    Instrumented (dev mode), goal: helping developers to find bugs


Syntastic
---------

By default vim will treat \*.h files as cpp, but syntastic has no make-checker
for cpp, so \*.h would not get checked.

.. code-block:: bash

   let g:syntastic_c_checkers = ['make']
   au BufNewFile,BufRead *.h set ft=c

With this setting syntastic will check the following:

* Clang-based build errors
* Line length
* Trailing whitespaces

Clang complete
--------------

If you use clang complete, we recommend

.. code-block:: vim

   let g:clang_auto_select     = 1
   let g:clang_snippets        = 1
   let g:clang_snippets_engine = 'clang_complete'

Thanks
======

For letting me do this:

* `Adfinis SyGroup`_

.. _`Adfinis SyGroup`: https://www.adfinis-sygroup.ch/

For helping me with the architecture:

* David Vogt @winged
* Sven Osterwalder @sosterwalder

For helping me with the documentation:

* Sven Osterwalder @sosterwalder
* David Vogt @winged

For reviewing my pull requests:

* Oliver Sauder @sliverc
* David Vogt @winged
* Tobias Rueetschi @keachi

License
=======

LGPL 3.0

Contributing
============

Please open an issue first. Contributions of missing features are very welcome, but
we want to keep to scope of libchirp minimal, so additional features should
probably be implemented in an upper layer.
