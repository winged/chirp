=====
C4irp
=====

Message-passing and actor-based programming for everyone

It is part of Concreteclouds_ and the C99 implementation of pychirp_.

.. _Concreteclouds: https://concretecloud.github.io/

.. _pychirp: https://github.com/concretecloud/pychirp

|floobits| |travis| |appveyor| |rtd| |coverage| [1]_

.. |floobits|  image:: https://floobits.com/ganwell/chirp.svg
   :target: https://floobits.com/ganwell/chirp/redirect
.. |travis|  image:: https://travis-ci.org/concretecloud/chirp-py.svg?branch=master
   :target: https://travis-ci.org/concretecloud/chirp-py
.. |appveyor| image:: https://ci.appveyor.com/api/projects/status/l8rw8oiv64ledar6?svg=true
   :target: https://ci.appveyor.com/project/ganwell/chirp
.. |rtd| image:: https://img.shields.io/badge/docs-master-brightgreen.svg
   :target: https://docs.adfinis-sygroup.ch/public/chirp/
.. |coverage| image:: https://img.shields.io/badge/coverage-100%25-brightgreen.svg

`Read the Docs`_

.. _`Read the Docs`: https://docs.adfinis-sygroup.ch/public/chirp/

.. [1] Coverage enforced by tests (on travis)

WORK IN PROGRESS
================

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

Install
=======


Install to prefix /usr/local.

.. code-block:: bash

   cd build
   ../configure
   make
   sudo make install

Install to prefix /usr.

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

Windows
-------

.. code-block:: bash

   python configure
   make.cmd
