// =================
// Quickcheck header
// =================
//
// Fork of https://github.com/mcandre/qc by Andrew Pennebaker removing gc.h
// and changing almost everything.
//
// Forked at commit fc8e7f76af339fdd56546ad46cb9d97cd42ec5cb
//
// We use quick-check style testing in chirp to mimic the hypothesis-based
// tests which are moved to a separate test-suite project.
//
// Syntax
// ======
//
// For simplicity, :c:func:`_ch_qc_for_all` is called using the
// :c:macro:`ch_qc_for_all` macro (see quickcheck.h).
//
// In this example, :c:macro:`ch_qc_for_all` is testing whether all integers
// are odd (they are not).
//
// Every :c:macro:`ch_qc_for_all` call begins with a property function. For
// example, is_odd takes an int and returns a bool.
//
// .. code-block:: cpp
//
//    int is_odd(ch_buf data);
//
// In order to handle arbitrarily-typed property functions, qc uses a special
// protocol, ch_qc_args,  to pass test values to the property function.
//
// .. code-block:: cpp
//
//    int is_odd(ch_buf data) {
//        int n = ch_qc_args(int, 0, int);
//
//        return n % 2 == 1;
//    }
//
// ch_qc_args' first argument is a type: int, char, int*, char*, int**, char**,
// ...  The second argument is the index (qc internally stores all test cases
// in a single array).  The final argument is the maximum byte size of the
// property's input types. For example, is_equal(short, int)'s maximum byte
// size would be sizeof(int).
//
// :c:macro:`ch_qc_for_all`'s first argument is the name of such a property
// function. The next argument is an array of generator functions.
//
// The is_odd property function has a single argument, an integer. Thus
// each test case consists of a random integer to be passed to the
// is_odd property function.  More complicated property functions (e.g.
// cmp(int, int)) may have multiple arguments, and therefore require multiple
// generators.
//
// .. code-block:: cpp
//
//    gen gs[] = { ch_qc_gen_int };
//
// When a test case fails, the values for which the property returns false will
// be printed. For each generator, a corresponding printer function is
// necessary. c:c:macro:h_qc_for_all is designed to test properties of arbitrary
// numbers of arbitrary types; because C has no universal print(some_object)
// function, the framework user must specify printer functions for each
// generator function.  More complicated types, such as trees, graphs, and
// linked lists require the framework user to write custom printer functions,
// but the syntax remains the same.
//
// .. code-block:: cpp
//
//    print ps[] = { ch_qc_print_int };
//
// Finally, c:c:macro:h_qc_for_all requires the maximum byte size of the types
// to be passed to the property function. This information helps
// c:c:macro:h_qc_for_all hold all test values in a single array, which it
// passes to the test property.
//
// .. code-block:: cpp
//
//    ch_qc_for_all(is_odd, 1, gs, ps, int);
//
// Please see :ref:`more-quickcheck-examples` for more quickcheck examples.
//
// .. code-block:: cpp

#ifndef ch_quickcheck_test_h
#define ch_quickcheck_test_h

// Project includes
// ================
//
// .. code-block:: cpp
//
#include "common.h"
#include "qs.h"
#include "rbtree.h"

// System includes
// ===============
//
// .. code-block:: cpp
//
#include <stdlib.h>
#include <stdbool.h>

// Declarations
// ============
//
// Types
// -----

// .. c:type:: ch_qc_mem_track_t
//
//    List to track memory allocations and free them after a test.
//
//    .. c:member:: ch_buf* data
//
//       Pointer to the data.
//
//    .. c:member:: size_t size
//
//       Size of one item of the memory
//
//    .. c:member:: unsigned char count
//
//       Count of items of the memory. size * count equals the total size in
//       bytes.
//
//    .. c:member:: struct ch_qc_mem_track_s *next
//
//       Pointer to the next list item.
//
// .. code-block:: cpp
//
typedef struct ch_qc_mem_track_s {
    ch_buf* data;
    size_t size;
    unsigned char count;
    struct ch_qc_mem_track_s *next;
} ch_qc_mem_track_t;

// rbtree prototypes
// -----------------
//
// .. code-block:: cpp

qs_stack_bind_decl_m(
    ch_qc,
    ch_qc_mem_track_t
)

// Macros
// ------
//
//
// .. c:macro:: ch_qc_args
//
//    Helper to access arguments passed to the property functions.
//
//    :param type type: The type of the argument.
//    :param size_t n: The index of the argument.
//    :param type max_class: The largest type returned by any generator.
//
// .. code-block:: cpp
//
#define ch_qc_args(type, n, max_class) \
    ((* (type*) ((void*) (data + n * sizeof(max_class)))))

// .. c:macro:: ch_qc_for_all
//
//   Helper macro instead of max_size just pass the biggest type that is
//   generated.
//
//    see: :c:func:`_ch_qc_for_all`
//
// .. code-block:: cpp
//
#define ch_qc_for_all(property, arglen, gs, ps, max_class) \
    (_ch_qc_for_all((ch_qc_prop) property, arglen, gs, ps, sizeof(max_class)))

// .. c:macro:: ch_qc_gen_array
//
//    Generate an array of a certain subtype. Quickcheck will track this
//    memory and free it for you when tests are done.
//
//    see: :c:func:`_ch_qc_gen_array`
//
//    :param ch_buf* data: Out parameter set to :c:type:`ch_qc_mem_track_t`\*
//                         which contains the requested data in its data field.
//    :param ch_qc_gen g: Generator for the subtype.
//    :param type class: Subtype generated by the generator.
//
// .. code-block:: cpp
//
#define ch_qc_gen_array(data, g, class) \
    (_ch_qc_gen_array(data, (ch_qc_gen) g, sizeof(class)))

// .. c:macro:: ch_qc_gen_return
//
//    Helper to implement the arguments protocol.
//
//    Casts the generated data to the given type and assigns the given value to
//    the data argument. Must be used inside a generator.
//
//    :param type type: The type the generator returns.
//    :param type value: The value of the type above, returned by the
//                       generator.
//
// .. code-block:: cpp
//
#define ch_qc_return(type, value) \
    ((* (type*) (void*) data) = value)

// .. c:macro:: ch_qc_tgen_array
//
//    Generate an array of a certain subtype. Quickcheck will track this
//    memory and free it for you when tests are done. Typed version used in
//    intermediate generation functions.
//
//    see: :c:func:`_ch_qc_tgen_array`
//
//    :param ch_qc_gen g: Generator for the subtype.
//    :param type class: Subtype generated by the generator.
//    :rtype: ch_qc_mem_track_t*
//
// .. code-block:: cpp
//
#define ch_qc_tgen_array(g, class) \
    (_ch_qc_tgen_array((ch_qc_gen) g, sizeof(class)))


// Types
// -----

// .. c:type:: ch_qc_gen
//
//    Type representing data generator functions
//
// .. code-block:: cpp
//
typedef void (*ch_qc_gen)(ch_buf*);

// .. c:type:: ch_qc_print
//
//    Type representing data print functions. Each generator must have a
//    corresponding print function.
//
// .. code-block:: cpp
//
typedef void (*ch_qc_print)(ch_buf*);

// .. c:type:: ch_qc_prop
//
//    Type representing property test functions. In quickcheck the test and
//    function to check if the property holds are one.
//
// .. code-block:: cpp
//
typedef int (*ch_qc_prop)(ch_buf*);

// Functions
// ---------

// .. c:function::
int
_ch_qc_for_all(
        ch_qc_prop property,
        size_t arglen,
        ch_qc_gen gs[],
        ch_qc_print ps[],
        size_t max_size
);
//
//    Generate 100 test-cases and tests for each case if a property holds.
//
//    In the protocol of quickcheck the generation of data is separated from
//    the test. If you are using the basic generator functions
//    ch_qc_gen[bool|char|int|string|array] quickcheck will track allocated
//    memory and free it for you.
//
//    see: :c:func:`ch_qc_for_all`
//
//    :param ch_qc_prop property: Function that runs the test and checks the
//                                property.
//    :param size_t arglen: The number of arguments passed to the property,
//                          also equals the number of generator- and
//                          print-functions.
//    :param ch_qc_gen gs[]: List of functions generating the input data.
//    :param ch_qc_print ps[]: List of functions able to print the associated
//                             generated data.
//    :param size_t max_size: The maximum of generated data across all
//                            generators.
//

// .. c:function::
void
_ch_qc_gen_array(
        ch_buf* data,
        ch_qc_gen g,
        size_t size
);
//    :noindex:
//
//    see: :c:macro:`ch_qc_gen_array`
//

// .. c:function::
ch_qc_mem_track_t*
_ch_qc_tgen_array(
        ch_qc_gen g,
        size_t size
);
//    :noindex:
//
//    see: :c:macro:`ch_qc_tgen_array`
//

// .. c:function::
void
ch_qc_free_mem(void);
//
//    Free memory allocations made during quickcheck test. Call this if you
//    don't intend to call ch_qc_for_all().

// .. c:function::
void
ch_qc_gen_bool(ch_buf* data);
//
//    Generate data of type bool. Has to be copied.
//
//    :param ch_buf* data: Out parameter, the bool as an int.

// .. c:function::
void
ch_qc_gen_byte(ch_buf* data);
//
//    Generate an binary character of type  uint8_t. Has to be copied.
//
//    :param ch_buf* data: Out parameter, the char.

// .. c:function::
void
ch_qc_gen_bytes(ch_buf* data);
//
//    Generate a string of bytes of type uint8_t*. The memoy allocated,
//    tracked and freed by quickcheck.
//
//    :param ch_buf* data: Out parameter set to :c:type:`ch_qc_mem_track_t`\*
//                         which contains the requested char* in its data field.

// .. c:function::
void
ch_qc_gen_char(ch_buf* data);
//
//    Generate an ascii character of type char. Has to be copied.
//
//    :param ch_buf* data: Out parameter, the char.

// .. c:function::
void
ch_qc_gen_double(ch_buf* data);
//
//    Generate data of type double. [0, 1]. Has to be copied.
//
//    :param ch_buf* data: Out parameter, the int.

// .. c:function::
void
ch_qc_gen_int(ch_buf* data);
//
//    Generate data of type int. Has to be copied.
//
//    :param ch_buf* data: Out parameter, the int.

// .. c:function::
void
ch_qc_gen_string(ch_buf* data);
//
//    Generate a string of ascii characters of type char*. The memoy allocated,
//    tracked and freed by quickcheck.
//
//    :param ch_buf* data: Out parameter set to :c:type:`ch_qc_mem_track_t`\*
//                         which contains the requested char* in its data field.
//

// .. c:function::
void
ch_qc_init(void);
//
//    Initializes srand. Can be omitted to initialize srand in different way.
//

// .. c:function::
void
ch_qc_print_bool(ch_buf* data);
//
//    Prints data of type bool.
//
//    :param ch_buf* data: The data.

// .. c:function::
void
ch_qc_print_byte(ch_buf* data);
//
//    Prints data of type ch_buf.
//
//    :param ch_buf* data: The data.

// .. c:function::
void
ch_qc_print_bytes(ch_buf* data);
//
//    Prints data of type ch_buf*.
//
//    :param ch_buf* data: The data.
//
// .. c:function::
void
ch_qc_print_double(ch_buf* data);
//
//    Prints data of type double.
//
//    :param ch_buf* data: The data.

// .. c:function::
void
ch_qc_print_char(ch_buf* data);
//
//    Prints data of type char.
//
//    :param ch_buf* data: The data.

// .. c:function::
void
ch_qc_print_int(ch_buf* data);
//
//    Prints data of type int.
//
//    :param ch_buf* data: The data.

// .. c:function::
void
ch_qc_print_string(ch_buf* data);
//
//    Prints data of type string.
//
//    :param ch_buf* data: The data.

// .. c:function::
ch_qc_mem_track_t*
ch_qc_tgen_bytes(void);
//
//    Generate a string of bytes of type uint8_t*. The memoy allocated, tracked
//    and freed by quickcheck. Typed version used in intermediate generation
//    functions.
//
//    :rtype: ch_qc_mem_track_t*
//

// .. c:function::
ch_qc_mem_track_t*
ch_qc_tgen_string(void);
//
//    Generate a string of ascii characters of type char*. The memoy allocated,
//    tracked and freed by quickcheck. Typed version used in intermediate
//    generation functions.
//
//    :rtype: ch_qc_mem_track_t*
//

// .. c:function::
ch_qc_mem_track_t*
ch_qc_track_alloc(size_t size);
//
//    Allocate memory and track it. The memory is freed by quickcheck.
//
//    :param size_t size: The size of memory to allocate
//

// Definitions
// ===========

// .. c:function::
static
ch_inline
int
ch_qc_tgen_bool(void)
//
//    Generate data of type bool.
//
//    :rtype: int
//
// .. code-block:: cpp
//
{
    return rand() % 2 == 0;
}

// .. c:function::
static
ch_inline
uint8_t
ch_qc_tgen_byte(void)
//
//    Generate byte of type uint8_t.
//
//    :rtype: uint8_t
//
// .. code-block:: cpp
//
{
    return rand();  // No need to mod overflow of unsigned char is
                    // is well defined.
}

// .. c:function::
static
ch_inline
char
ch_qc_tgen_char(void)
//
//    Generate ascii character of type char.
//
//    :rtype: char
//
// .. code-block:: cpp
//
{
    return (char)((rand() % 127) + 1);
}


// .. c:function::
static
ch_inline
double
ch_qc_tgen_double(void)
//
//    Generate positive float of type double.
//
//    :rtype: dobule
//
// .. code-block:: cpp
//
{
    return ((double) rand() / (double) RAND_MAX);
}

// .. c:function::
static
ch_inline
uint8_t
ch_qc_tgen_int(void)
//
//    Generate int.
//
//    :rtype: dobule
//
// .. code-block:: cpp
//
{
    return rand();
}

// .. code-block:: cpp

#endif //ch_quickcheck_test_h
