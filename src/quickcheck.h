// =================
// Quickcheck header
// =================
//
// Fork of https://github.com/mcandre/qc by Andrew Pennebaker removing gc.h and
// other evil things.
//
// Forked at commit fc8e7f76af339fdd56546ad46cb9d97cd42ec5cb
//
// We use quick-check style testing in chirp to mimic the hypothesis-based
// tests which are moved to a separate test-suite project.
//
// Syntax
// ======
//
// For simplicity, _for_all is called using the for_all macro (see qc.h).
//
// In this example, for_all is testing whether all integers are odd (they are
// not).
//
// Every for_all call begins with a property function. For example, is_odd
// takes an int and returns a bool.
//
// .. code-block:: cpp
//
//    bool is_odd(blob data);
//
// In order to handle arbitrarily-typed property functions, qc uses a special
// protocol, qc_args,  to pass test values to the property function.
//
// .. code-block:: cpp
//
//    bool is_odd(blob data) {
//        int n = qc_args(int, 0, int);
//
//        return n % 2 == 1;
//    }
//
// qc_args' first argument is a type: int, char, int*, char*, int**, char**,
// ...  The second argument is the index (qc internally stores all test cases
// in a single array).  The final argument is the maximum byte size of the
// property's input types. For example, is_equal(short, int)'s maximum byte
// size would be sizeof(int).
//
//
// for_all's first argument is the name of such a property function. The next
// argument is an array of generator functions. The is_odd property function
// has a single argument, an integer. Thus each test case consists of a random
// integer to be passed to the is_odd property function. More complicated
// property functions (e.g. cmp(int, int)) may have multiple arguments, and
// therefore require multiple generators.
//
// .. code-block:: cpp
//
//    gen gs[] = { gen_int };
//
// When a test case fails, the values for which the property returns false will
// be printed. For each generator, a corresponding printer function is
// necessary.  for_all is designed to test properties of arbitrary numbers of
// arbitrary types; because C has no universal print(some_object) function, the
// framework user must specify printer functions for each generator function.
// More complicated types, such as trees, graphs, and linked lists require the
// framework user to write custom printer functions, but the syntax remains the
// same.
//
// .. code-block:: cpp
//
//    print ps[] = { print_int };
//
// Finally, for_all requires the maximum byte size of the types to be passed to
// the property function. This information helps for_all hold all test values
// in a single array, which it passes to the test property.
//
// .. code-block:: cpp
//
//    for_all(is_odd, 1, gs, ps, int);
//
// .. code-block:: cpp

#ifndef ch_quickcheck_h
#define ch_quickcheck_h

#include <stdlib.h>
#include <stdbool.h>

void qc_init(void);

#define qc_return(type, value) ((* (type*) data) = value)
#define qc_args(type, n, max_class) ((* (type*) (data + n * sizeof(max_class))))

typedef void* blob;

typedef void (*gen)(blob);
typedef void (*print)(blob);
typedef bool (*prop)(blob);

void gen_bool(/*@out@*/ blob const data);
void gen_int(/*@out@*/ blob const data);
void gen_char(/*@out@*/ blob const data);

void _gen_array(/*@out@*/ blob const data, gen const g, size_t const size);

#define gen_array(data, g, class) (_gen_array(data, (gen) g, sizeof(class)))

void gen_string(/*@out@*/ blob const data);

void print_bool(blob const data);
void print_int(blob const data);
void print_char(blob const data);
void print_string(blob const data);

bool _for_all(
  prop const property,
  size_t const arglen,
  gen const gs[],
  print const ps[],
  size_t const max_size
);

#define for_all(property, arglen, gs, ps, max_class) \
  (_for_all((prop) property, arglen, gs, ps, sizeof(max_class)))

#endif //ch_quickcheck_h
