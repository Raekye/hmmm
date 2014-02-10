#include "node.h"
#include "parser.hpp"
#include "codegen.h"

#include <cstdlib>
#include <cstdio>
#include <iostream>

int yyparse(NExpression** expression, yyscan_t scanner);

extern NExpression* programBlock;

int main() {
	//yyparse();
	std::cout << programBlock << std::endl;

	CodeGenContext context;
	context.generate_code(programBlock);
	context.run_code();
	return 0;
}