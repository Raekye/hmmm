FILES = lexer.cpp parser.cpp main.cpp node.cpp codegen.cpp codescope.cpp
CC = g++
CFLAGS = -g -D__STDC_CONSTANT_MACROS -D__STDC_LIMIT_MACROS

lexer.cpp: lexer.l
	flex -o lexer.cpp --header-file=lexer.h lexer.l

parser.cpp: parser.y lexer.cpp
	bison -d -o  parser.cpp parser.y

build: $(FILES)
	$(CC) $(CFLAGS) $(FILES) -o calc `llvm-config --libs core jit native --cxxflags --ldflags`

clean:
	rm -f *.o *~ lexer.cpp lexer.h parser.cpp parser.hpp calc

kaleidoscope:
	clang++ -g kaleidoscope.cpp `llvm-config --cppflags --ldflags --libs core jit native` -rdynamic -O3 -o kaleidoscope.o

playground:
	g++ playground.cpp -o playground.o