
First tests
===========

At this stage, you have all the components to build a simple monitoring system.
Run the following commands in separate shells to see what's happening:

.. code-block:: bash

   # First, start the monitoring server itself.
   ./monitor 9998

.. code-block:: bash

   # Start monitoring some random web server
   ./agent example.org:80 30 localhost:9998

.. code-block:: bash

   # Monitor another web server
   ./agent google.com:80 60 localhost:9998


Now watch what's happening on the monitor's output:

.. code-block:: text

   Service         Last update           Interval  State
   example.org:80  2017-11-09 14:35:05   60s       OK
   google.com:80   2017-11-09 14:35:33   60s       OK

