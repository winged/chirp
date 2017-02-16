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
.. |rtd| image:: https://img.shields.io/badge/docs-master-brightgreen.svg
   :target: http://1042.ch/chirp/
.. |coverage| image:: https://img.shields.io/badge/coverage-100%25-brightgreen.svg

`Read the Docs`_

.. _`Read the Docs`: http://1042.ch/chirp/

.. [1] Python/hypothesis based test-suite in external project seecc_.

.. [2] Coverage enforced by tests (on travis, after 1.0 release)

.. _seecc: https://github.com/concretecloud

WORK IN PROGRESS
================

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
   sudo make install

Install to prefix /usr. (without docs)

.. code-block:: bash

   cd build
   ../configure --prefix /usr
   make
   sudo make install

In-source build is also possible.

.. code-block:: bash

   ./configure
   make
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

Unix
----

.. code-block:: bash

   cd build
   ../configure --dev
   make test

In development mode the make file has a help:

.. code-block:: bash

   make


Windows
-------

No development build available.

License
=======

LGPL 3.0
