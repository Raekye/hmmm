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
	llvm::InitializeNativeTarget();
	NExpression* root_expr = getAST("3 * 7 - 2");
	if (root_expr == NULL) {
		std::cout << "Root expression was null! Ahhhhhhhhhhhhh!" << std::endl;
		return 1;
	}

	std::string error_str;
	llvm::Module* module = new llvm::Module("top", llvm::getGlobalContext());
	llvm::ExecutionEngine* execution_engine = llvm::EngineBuilder(module).setErrorStr(&error_str).setEngineKind(llvm::EngineKind::JIT).create();
	CodeGenContext context(module);

	std::cout << "execution engine " << execution_engine << std::endl;
	if (execution_engine == NULL) {
		std::cout << "Unable to create execution engine." << std::endl;
		return 1;
	}

	NFunction main_fn(root_expr);

	llvm::Value* root_val = root_expr->gen_code(&context);

	std::cout << "Root val code:" << std::endl;
	root_val->dump();

	llvm::Function* main_fn_val = (llvm::Function*) main_fn.gen_code(&context);
	std::cout << "Main fn code:" << std::endl;
	main_fn_val->dump();
	void* fn_ptr = execution_engine->getPointerToFunction(main_fn_val);
	int64_t (*fn_ptr_native)() = (int64_t (*)())(intptr_t) fn_ptr;
	std::cout << "Main fn at " << fn_ptr << "; executed: " << fn_ptr_native() << std::endl;

	// context.generate_code(programBlock);
	// context.run_code();
	return 0;
}