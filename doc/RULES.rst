=====
RULES
=====

* Mantra
   - Make it work
   - Make it 100% tested
   - Make it to be used by people
   - Make it fast (only if really needed)
   - Make it beautiful
* Log and assert a lot
* The following are our given abstractions:

  - libuv
  - sglib
  - openssl
  - We try not do add our own abstractions, to keep complexity low

    - For example we use a async-semaphore pattern to await multiple callbacks,
      it would be possible to build a abstraction (API) from this, but it would
      increase complexity. So unless we need this async-semaphore in 20+ places we
      just repeat the pattern, which keeps flexibility high and complexity low.

* We use defined length integers ie. uint8_t for file-formates, wire-protocols
  and when plain int is really really wasteful
* Buffers and chars:
  * char* for C-strings
  * void* for buffers (in interfaces)
  * ch_buf* for buffer-instances (an alias for char*)
  * uint8_t for bytes (for example the identity)
* Use one flags member instead of many bools (int)
* Structs end in _s
* All the structs have a typedef ending in _t
* Types end in _t
* Callbacks end in _cb

  - Use natural names and _cb will indicated that it probably happened AFTER

* Callback types end in _cb_t
* Sort symbols alphabetically including underscores "_"
  - When independent

* Forward declarations go to common.h. Example

.. code-block:: cpp

   struct ch_chirp_s;
   typedef struct ch_chirp_s ch_chirp_t;

* Basic layout, follow where feasible

.. code-block:: text

   internal includes

   external includes

   defines

   other symbols (sorted, when independent)

   function symbols (sorted)

* Indent ifdef

.. code-block:: cpp

   #ifdef _WIN32
   #   if defined(_MSC_VER) && _MSC_VER < 1600
   #       include <stdint-msvc2008.h>
   #       define ch_inline __inline
   #   else // _MSC_VER
   #       include <stdint.h>
   #       define ch_inline inline
   #   endif // _MSC_VER
   #endif //_WIN32

* Always unpack handles in functions and callbacks till you can verify the chirp magic
* Chirp and C3irp are only used to disambiguate the python-only and the C based
  version of chirp, everywhere else chirp is used
* Embeddable: allocate memory via user callback
* Every function returns ch_error_t
* Use pointers sparsely

  - Copy small structs 
  - Use pointers for large structs
  - Use pointer if it has to be modified (also out params)
  - Use pointer where you have to because of forward declarations

* Embrace libuv styles and use it for chirp API
* Literate programming
* Local messages are sent to scheduler directly
  - Binding will send local messages to scheduler directly (not using chirp)
* Localhost connections bypass TLS
* Use C99 plus the extension used by libuv
* PEP8 style in C is ok
* Sending messages my not allocate memory

  - Only things that happen seldom may allocate
  - Luckily chirp is already design that way

* The original chirp API may only be slightly changed
* It must be possible for original chirp to adapt the new wire protocol

  - So we have a pure-python and C implementation

* Provide wheels
* Provide distro packages

Performance
===========

* Adding buffering per connection would destroy some of the nice properties of
  chirp, mainly flow-control, simpleness and robustness.

   - Therefore we do not ever allow to remove the per connection send-lock,
     which means only one message can be sending and the next message can only
     be sent after the current message has been acknowledged. The first
     statement is important for simpleness and robustness and the second
     statement makes flow-control possible.

   - Since we the error condition sent to the user is a timeout on the ack. We
     can react on all other errors accordingly, but do not have to report back
     to the user. Which saves extremely complex callback structures. Yey!

* Since chirp is meant for multiprocessing, our performance goals refer to this
  configuration

   - x must be able to send/receive 300'000+ message to/from a suitable N peers

.. graphviz::

   digraph FAST {
      concentrate=true;
      x -> a;
      x -> b;
      x -> c;
      x -> d;
      x -> e;
      x -> f;
      x -> N;
      a -> x;
      b -> x;
      c -> x;
      d -> x;
      e -> x;
      f -> x;
      N -> x;
   }

* For this configuration we just have to beat 10'000 messages of course the
  more the better

.. graphviz::

   digraph FAST {
      concentrate=true;
      x -> a;
      a -> x;
   }

* Of course 300'000 msg is our stretch goal, 30'000 msg is ok too

=========
Questions
=========

Thing that aren't rules yet
