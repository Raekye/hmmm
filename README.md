hmmm
====
Me playing around, learning to build compilers.
See my [compiler for a custom soft processor on an FPGA][1] for this put in practice.

## Directory map
Directories `madoka` to `tk` contain a `Makefile` that puts stuff in a `bin/` folder.
Starting from `midori`, CMake is used.
Example build: `mkdir build && cd build && cmake .. && make && make test`.

- `madoka`: one of my first passes, arguably my first success I could call a "compiler". Pre 2014-summer
- `sayaka`: successor to `madoka`, had ideas on what to do differently. The ideas were pre 2014-summer, most of the work on it is post 2014-summer
- `siyu`: hand written LL(1) regex parser, NFA state generator, DFA state generator, lexer-generator, and parser-generator
- `tk`: successor to `siyu`, completed LALR(1) parser generator
- `midori`: successor to `tk`, fixed/much improved LALR(1) parser generator, starting code gen

### Siyu
- hand written, recursive descent basic regex parser (builds AST)
- regex used to define tokens, lexer-generator generates states and next-states for a lexer (a big FSM)
- generate DFA states from NFA states
- `siyu-1` is a start at generating DFA states directly from a regex based on the dragon book

### Tk
- fixed parser handling epsilon productions

### Midori
- lexer/finite automata now support ranges using interval trees
- rewrote parser generator/parsing algorithm several times:
	- [SLR(1)][2]
	- [LR(1)][3]
	- [LALR(1)][4] based on the dragon book
	- [LALR(1)][5] using DeRemer and Pennello's [lookahead algorithm][6], based on [PLY][7],
		and as described in The Theory and Practice of Compiler Writing, and Parsing Theory Volume 2
- re-implemented calculator in `madoka`

## Regex grammar
- multiplication is repetition
- addition is concatenation
- multiplication, addition, and logical or have the same precedence as they usually would in a programming language (in that order)

```
TOKEN_STAR: *
TOKEN_PLUS: +
TOKEN_QUESTION_MARK: ?
TOKEN_OR: |
TOKEN_ESCAPE: \

TOKEN_LPAREN: (
TOKEN_RPAREN: )
TOKEN_LBRACE: {
TOKEN_RBRACE: }
TOKEN_LBRACKET: [
TOKEN_RBRACKET: ]

TOKEN_SPECIAL: any of the tokens above
TOKEN_PLAIN: everything not TOKEN_SPECIAL, code point in [32 (space) , 127 (tilda) )
TOKEN_GROUP_SPECIAL: TOKEN_LBRACKET | TOKEN_RBRACKET | TOKEN_DASH | TOKEN_ESCAPE
TOKEN_GROUP_PLAIN: everything not TOKEN_GROUP_SPECIAL, code point in [32, 127)

TOKEN_DASH: -
TOKEN_COMMA: ,

TOKEN_X: x
TOKEN_U: u
TOKEN_T: t
TOKEN_N: n
TOKEN_R: r

TOKEN_HEX_DIGIT: [0-9a-f]
TOKEN_DEC_DIGIT: [0-9]

top_level
	: lr_or
	;

lr_or
	: not_lr_or TOKEN_OR lr_or
	| not_lr_or
	;

not_lr_or
	: lr_add
	;

lr_add
	: not_lr_add lr_add
	| not_lr_add
	;

not_lr_add
	: lr_mul
	;

lr_mul
	: not_lr_mul TOKEN_STAR
	| not_lr_mul TOKEN_PLUS
	| not_lr_mul TOKEN_QUESTION_MARK
	| not_lr_mul mul_range
	| not_lr_mul
	;

not_lr_mul
	: not_lr
	;

not_lr
	: parentheses
	| literal
	| group
	;

mul_range
	: TOKEN_LBRACE dec_int TOKEN_COMMA dec_int TOKEN_RBRACE
	;

parentheses
	: TOKEN_LPAREN top_level TOKEN_RPAREN
	;

literal
	: absolute_literal
	| TOKEN_ESCAPE TOKEN_SPECIAL
	| TOKEN_PLAIN
	;

group
	: TOKEN_LBRACKET group_contents TOKEN_RBRACKET
	;

group_contents
	: group_element group_contents
	| group_element
	;

group_element
	: group_range
	| group_literal
	;

group_literal
	| absolute_literal
	| TOKEN_ESCAPE TOKEN_GROUP_SPECIAL
	| TOKEN_GROUP_PLAIN
	;

group_range
	: group_literal TOKEN_DASH group_literal
	;

absolute_literal
	: TOKEN_ESCAPE TOKEN_X hex_byte
	| TOKEN_ESCAPE TOKEN_U hex_int
	| TOKEN_ESCAPE TOKEN_T
	| TOKEN_ESCAPE TOKEN_N
	| TOKEN_ESCAPE TOKEN_R
	;

hex_byte
	: TOKEN_HEX_DIGIT TOKEN_HEX_DIGIT
	;

hex_int
	: hex_byte hex_byte hex_byte hex_byte
	;

dec_int
	: TOKEN_DEC_DIGIT
	| TOKEN_DEC_DIGIT dec_int
	;
```

## Dependencies
- gcc-c++
- llvm-devel
- llvm-static
- boost-devel
- flex
- bison
- gdb (debug)
- valgrind (debug)
- lcov (coverage)

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
- http://swtch.com/~rsc/regexp/regexp1.html
- http://stackoverflow.com/questions/2245962/is-there-an-alternative-for-flex-bison-that-is-usable-on-8-bit-embedded-systems
- https://web.cs.dal.ca/~sjackson/lalr1.html
- https://stackoverflow.com/questions/8242509/how-does-the-yacc-bison-lalr1-algorithm-treat-empty-rules
- https://stackoverflow.com/questions/57120176/grammar-matching-regex-character-classes-trailing-dash/
- Compilers: Principles, Techniques, and Tools (the dragon book)
- Parsing Theory Volume 2: LR(k) and LL(k) Parsing
- The Theory and Practice of Compiler Writing
- Efficient Computation of LALR(1) Look-Ahead Sets, DeRemer and Pennello (1982) ([link][6])
- Efficient Parsing for Natural Language: A Fast Algorithm for Practical Systems
- http://scottmcpeak.com/elkhound/elkhound.ps
- https://web.stanford.edu/class/archive/cs/cs143/cs143.1128/

[1]: https://github.com/Raekye/bdel_and_dfr_compiler
[2]: https://github.com/Raekye/hmmm/tree/1130d9626c838b36b54155926df05da25e4e828f/midori/src/midori/parser.cpp
[3]: https://github.com/Raekye/hmmm/tree/ca9659d56b1876f5a325463ebcdb04aec0e3cfbe/midori/src/midori/parser.cpp
[4]: https://github.com/Raekye/hmmm/tree/a4cb4c7e844ef49d675a9faac622d8d57c8da184/midori/src/midori/parser.cpp
[5]: https://github.com/Raekye/hmmm/tree/b0b7932e6c7ba5db770fd2ebe5ea3c5b6bfe0a79/midori/src/midori/parser.cpp
[6]: https://dl.acm.org/citation.cfm?id=357187
[7]: https://github.com/dabeaz/ply
