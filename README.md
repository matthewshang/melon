# melon

melon is a dynamically typed programming language implemented in C with no external dependencies. The syntax is inspired by JavaScript; a collection of code samples can be found under the [test](test/) directory, including a [backtracking Sudoku solver](test/sudoku.txt).

Here is a list of noteworthy features:
* multipass compiler that emits a custom bytecode
* stack-based virtual machine
* classes: objects, static variables, constructors, operator overloading
* first class functions and closures
* lexical scope
* recursive descent parsing
* builtin datatypes: arrays, ranges, strings, hash tables
* helpful error reporting

## Compiling
Clone the repository, run CMake in the desired build directory to generate a Makefile, and the rest should work automatically. Although melon has only been tested on Ubuntu, it should work on any platform with a C11 compiler. By default, melon builds in release mode; add `-DCMAKE_BUILD_TYPE=Debug` for a debug build. There are no external dependencies.

## Helpful Links
The overall design of melon is based on the Gravity programming language and Crafting Interpreters. Here are some more resources I found helpful while writing melon: 
* [Gravity](https://github.com/marcobambini/gravity)
* [Crafting Interpreters](https://craftinginterpreters.com/)
* [More on interpreters](https://ruslanspivak.com/lsbasi-part12/)
* [Lua design](http://www.lua.org/doc/jucs05.pdf)
* [Precedence parsing](http://journal.stuffwithstuff.com/2011/03/19/pratt-parsers-expression-parsing-made-easy/)
* [Virtual machines](http://andreabergia.com/stack-based-virtual-machines-5/)
* [MurmurHash](https://en.wikipedia.org/wiki/MurmurHash)
* [Metaclasses](http://hokstad.com/compiler/43-eigenclasses)
