=====
Chirp
=====

Message-passing and actor-based programming for everyone

It is part of Concreteclouds_ and the C99 implementation of pychirp_.

.. _Concreteclouds: https://concretecloud.github.io/

.. _pychirp: https://github.com/concretecloud/pychirp

|travis| [1]_ |rtd| |coverage| [2]_

.. |travis|  image:: https://travis-ci.org/concretecloud/chirp.svg?branch=master
   :target: https://travis-ci.org/concretecloud/chirp
.. |rtd| image:: https://1042.ch/ganwell/docs-master.svg
   :target: https://1042.ch/chirp/
.. |coverage| image:: https://1042.ch/ganwell/coverage-100.svg

`Read the Docs`_

.. _`Read the Docs`: http://1042.ch/chirp/

.. [1] Python/hypothesis based test-suite in external project seecc_.

.. [2] Coverage enforced by tests (on travis, after 1.0 release)

.. _seecc: https://github.com/concretecloud

WORK IN PROGRESS
================

.. image:: https://graphs.waffle.io/concretecloud/chirp/throughput.svg 
 :target: https://waffle.io/concretecloud/chirp/metrics/throughput 
 :alt: 'Throughput Graph'

Features
========

* Fully automatic connection setup

* TLS support

  * Connections to 127.0.0.1 and ::1 aren't encrypted

* Flow control

  * Chirp won't overload peers out-of-the box, if you work with long requests
    >2.5s adjust the timeout
  * Peer-load is reported so you can implement load-balancing easily

* Easy message routing

* Robust

  * No message can be lost without an error (or it is a bug)
  * Due to retry it takes a very bad network for messages to be lost

* Very thin API

* Minimal code-base, all additional features will be implemented as modules in
  an upper layer

* Fast

  * Up to 50'000 msg/s on a single connection
  * Up to TODO msg/s in star topology
  * Using multiple channels multiplies throughput until another bottle-neck
    kicks in

Install
=======

Unix
----

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

In-source build is also possible.

.. code-block:: bash

   ./configure
   make
   make check
   sudo make install

.. [3] Script-headers can be patched to work with python2. It also possible to generate
   the makefile on a different system (for example in a embedded scenario)

Windows
-------

* Install Visual Studio 2015 if you want to build for python 3

  * Select Git for Windows in Visual Studio Installer or install it

* Install Visual Studio_ 2008 Express

* Install Windows SDK_ for Windows Server 2008 and .NET Framework 3.5 (python 2)

.. _Studio: http://download.microsoft.com/download/E/8/E/E8EEB394-7F42-4963-A2D8-29559B738298/VS2008ExpressWithSP1ENUX1504728.iso

.. _SDK: http://www.microsoft.com/en-us/download/details.aspx?id=24826

.. NOTE::

   To support *python 2.7 == Visual Studio 2008*, we use c99conv to convert to c89
   and we support *python 3.5+ == Visual Studio 2015* without c99conv. Any future
   version of python that uses a newer version of Visual Studio has to added to
   the build-system explicitly.

.. code-block:: bash

   python configure
   make.cmd

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

Please memcheck your code, we haven't automated memcheck, yet:

.. code-block:: bash

   valgrind --tool=memcheck ./src/[relevant]_etest

Docker
------

If a tool is not available on your platform or you have a old version of
cppcheck (cppcheck is known to behave very different across versions), you can
use the docker based tests.

.. code-block:: bash

   ./ci/alpine.sh

Travis will also run this script, so you can also use it to reproduce errors on
travis.


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

.. code-block:: bash

   let g:clang_auto_select     = 1
   let g:clang_snippets        = 1
   let g:clang_snippets_engine = 'clang_complete'

License
=======

AGPL 3.0

Contribution
============

1. Ask first

2. You have to agree, that we are free to change the license.
