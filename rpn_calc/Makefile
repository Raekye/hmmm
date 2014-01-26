FILES = lexer.c parser.c expression.cpp main.c
CC = g++
CFLAGS = -g -ansi

lexer.c: lexer.l
	flex lexer.l

parser.c: parser.y lexer.c
	bison parser.y

build: $(FILES)
	$(CC) $(CFLAGS) $(FILES) -o rpn_calc

clean:
	rm -f *.o *~ lexer.c lexer.h parser.c parser.h rpn_calc