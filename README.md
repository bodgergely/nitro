# nitro - the blazingly fast compressor from the future

Implemented in C++ nitro has the convenience of a standalone application but offers a library interface for its powerusers.

Unparalleled speed/conversion ratio!

nitro currently supports the simple 'block encoding' scheme.

## Design decisions

Block encoding is simple to implement and offers good (but not the best by far) compression ratio if the
number of unique symbols (8 bit value) is equal or less than 16. 

#### Algorithm:
	- Count the number of unique symbols (8 bit) values in the data
	- Use ceil(log2(count_unique_syms)) bits to represent each of those symbols
	- Build a symbol table which associates the symbols with their encodings (prepend to the output)
	- Iterate over the input and encode the symbols

it also offers a rather straightforward way to work in constant time to
'index into' the compressed representation to retrieve an original symbol.
This is possible because we are using identical block size (number of bits) to encode
every symbol. 

Arithmetic encoding support offers better compression ratios but more complicated to
implement and very quickly degrades if the number of unique characters are large.
It is the next candidate for encoding scheme to be implemented in nitro.

The second BIG design constraint is the need to keep the whole input data in memory as nitro
does the compression in memory without flushing to disk. This has the advantage of a rather 
simplified and flexible interface for users and is not filestream bound.


## Platform support
- Linux/Windows


## Install

#### Build all

Run build.sh to build everything. 

- shared library (libnitro.so)
- test driver (testNitro)
- nitro app (nitro)

/bin and /lib will contain the binaries.

#### Build separately

- build_lib.sh
- build_tests.sh
- build_app.sh

## Tests

Google Test unit tests.

After building the library and test driver you can run ./run_tests.sh

## Requirements

- GTest
- gcc 7.3.0 or Visual Studio 2017 or any C++17 supporting compiler (C++14 should work too)

### LD_LIBRARY_PATH should be set when using non system shared lib location.

(You can export LD_LIBRARY_PATH into your environment too for convenience)

## example: 

LD_LIBRARY_PATH=./lib ./bin/nitro -c genome.txt compressed.txt


# Licence
MIT

TODO
====

- Set up cmake/premake/make and test on linux
- Write Google Tests
- Clean up includes
- exception handling
- split encoder and decoder up from .hpp to both .hpp and .cpp
- Separation of client headers from implementation files
- Clean up compiler warnings
- Brute force memleak test - encoding/decoding to see if we have some memory leak


