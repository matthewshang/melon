# melon

## About
Melon is a small stack-based programming language that I made to learn about the inner workings of compilers. It compiles to a custom bytecode, which is then run by the melon virtual machine. Although it is not quite complete, you can find example programs that it can successfully 
compile and run in the test folder, including a brute force sudoku solver.

Here is hello world in melon:
```
println("Hello, World!");
```

I was inspired to write this after seeing the Gravity language on github. The overall design of melon is based on Gravity, although the
implementation is different. This project has been a great learning experience for me, especially since going into it I had no idea
how programming languages worked on the inside.

## Links
Here are some resources I found helpful while writing melon.

https://github.com/marcobambini/gravity <br />
interpreters: https://ruslanspivak.com/lsbasi-part12/ <br />
lua design: http://www.lua.org/doc/jucs05.pdf <br />
precedence parsing: http://journal.stuffwithstuff.com/2011/03/19/pratt-parsers-expression-parsing-made-easy/ <br />
vm: http://andreabergia.com/stack-based-virtual-machines-5/ <br />
hash: https://en.wikipedia.org/wiki/MurmurHash <br />
metaclasses: http://hokstad.com/compiler/43-eigenclasses <br />
