
Project home and documentation
==============================

`Project home`_

.. _`Project home`: https://github.com/concretecloud/chirp

`Read the docs`_

.. _`Read the docs`: http://1042.ch/chirp/

How to build and install
========================

By default it will be installed in /usr/local.

Example: Install in /usr

.. code-block:: bash

   make
   sudo make install PREFIX=/usr

Example: Packaging

.. code-block:: bash

   make
   make install PREFIX=/usr DEST=./pkgdir
