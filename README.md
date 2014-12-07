hmmm
====

## About
Me playing around, learning to build compilers.
The compiler parses some (custom) language, but I'm not documenting that because this project focuses on compiler design, not language design.
The grammar is always changing.
What I can actually compile and run is not very impressive, but I think the source code - handling language constructs - is interesting.
See my [compiler for a custom soft processor on a FPGA][1] for this put in practice (that project also shows lower-level code generation; this project uses LLVM for that).

## Directory map
The names are not particularly meaningful; just something unique and identifiable (not "lexer" or "compiler_1") I thought of at the moment.
Each of these directories has a `Makefile` that puts stuff in a `bin/` folder.

- `madoka/`: one of my first passes, arguably my first success I could call a "compiler". Pre 2014-summer
- `sayaka/` (WIP): successor to `madoka/`, had ideas on what to do differently. The ideas were pre 2014-summer, most of the work on it is post 2014-summer
- `primed/` (WIP): lexer-generator and parser-generator

## Dependencies
- gcc-c++
- llvm-devel
- llvm-static
- boost-devel
- flex
- bison
- gdb (debug)
- valgrind (debug)

## TODO
- function types

## Flex and Bison stuff
- http://flex.sourceforge.net/manual/
	- http://flex.sourceforge.net/manual/Reentrant.html
	- http://flex.sourceforge.net/manual/Scanner-Options.html
	- http://flex.sourceforge.net/manual/Index-of-Scanner-Options.html
	- http://flex.sourceforge.net/manual/Matching.html
- https://www.gnu.org/software/bison/manual/html_node/index.html
	- https://www.gnu.org/software/bison/manual/html_node/Grammar-File.html
	- https://www.gnu.org/software/bison/manual/html_node/Declarations.html (see "%define Summary", "%define api.pure")
	- https://www.gnu.org/software/bison/manual/html_node/Pure-Decl.html
	- https://www.gnu.org/software/bison/manual/html_node/Pure-Calling.html
	- https://www.gnu.org/software/bison/manual/html_node/Error-Reporting.html
	- https://www.gnu.org/software/bison/manual/html_node/Interface.html

## Resources
- http://stackoverflow.com/questions/3104389/can-i-bind-an-existing-method-to-a-llvm-function-and-use-it-from-jit-compiled-c
- http://stackoverflow.com/questions/3551733/llvm-automatic-c-linking
- http://stackoverflow.com/questions/4425797/linking-llvm-jit-code-to-external-c-functions
- http://stackoverflow.com/questions/14307906/c-llvm-class-functionality

[1]: https://github.com/Raekye/bdel_and_dfr_compiler
