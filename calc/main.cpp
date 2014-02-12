#include "node.h"
#include "parser.hpp"
#include "codegen.h"
#include "lexer.h"

#include <cstdlib>
#include <cstdio>
#include <iostream>

int yyparse(NExpression** expression, yyscan_t scanner);

extern NExpression* programBlock;

NExpression* getAST(const char* str) {
	NExpression* expr;
	yyscan_t scanner;
	YY_BUFFER_STATE state;
	if (yylex_init(&scanner)) {
		return NULL;
	}

	state = yy_scan_string(str, scanner);

	if (yyparse(&expr, scanner)) {
		return NULL;
	}

	yy_delete_buffer(state, scanner);
	yylex_destroy(scanner);

	return expr;
}

int main() {
	// llvm::InitializeNativeTarget();
	NExpression* root_expr = getAST("3 + 4");
	if (root_expr == NULL) {
		std::cout << "Root expression was null" << std::endl;
		return 1;
	}

	CodeGenContext context;

	llvm::Value* root_val = root_expr->gen_code(&context);

	std::cout << "Root val code:" << std::endl;
	root_val->dump();

	llvm::Module* module = new llvm::Module("top", llvm::getGlobalContext());
	llvm::ExecutionEngine* execution_engine = llvm::EngineBuilder(module).create();

	// context.generate_code(programBlock);
	// context.run_code();
	return 0;
}