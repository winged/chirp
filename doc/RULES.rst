=====
RULES
=====

* Mantra

  - Make it work
  - Make it 100% tested
  - Make it to be used by people
  - Make it fast (only if really needed)
  - Make it beautiful

* Do not use static functions in headers.
* Log and assert a lot
* Use C style comments. // is reserved for restructured text.
* The following are our given abstractions:

  - libuv
  - rbtree
  - openssl
  - sds (for tests only)
  - We try not do add our own abstractions, to keep complexity low

* We use defined length integers ie. uint8_t for file-formates, wire-protocols
  and when plain int is really really wasteful
* Buffers and chars:

  - char* for C-strings
  - void* for buffers (in interfaces)
  - ch_buf* for buffer-instances (an alias for char*)
  - uint8_t for bytes (for example the identity)

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

   #ifdef LIBRESSL_VERSION_NUMBER
   #   define CH_OPENSSL_10_API
   #   define CH_LIBRESSL
   #else
   #   define CH_OPENSSL
   #   if (OPENSSL_VERSION_NUMBER <= 0x10100000L)
   #       define CH_OPENSSL_10_API
   #   endif
   #endif

* Always unpack handles in functions and callbacks till you can verify the chirp magic
* Embeddable: allocate memory via user callback
* Embrace libuv styles and use it for chirp API
* Literate programming (kinda)
* Localhost connections bypass TLS
* Use C99
* PEP8 style in C is ok
* Tests don't have to be documented, so people can write tests fast and in flow.
* Sending messages my not allocate memory

  - Only things that happen seldom may allocate
  - Luckily chirp is already designed that way

* Provide distro packages

=========
Questions
=========

Things that aren't rules yet
