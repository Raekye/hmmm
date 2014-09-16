#include "node.h"
#include "parser.h"
#include "codegen.h"
#include "lexer.h"

#include <cstdlib>
#include <cstdio>
#include <iostream>
#include <fstream>

#include <boost/throw_exception.hpp>

#ifdef BOOST_NO_EXCEPTIONS
namespace boost {
	void throw_exception(const std::exception& e) {
		throw e;
	}
}

#endif // BOOST_NO_EXCEPTIONS

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

void run_code(const char* code) {
	NExpression* root_expr = getAST(code);
	if (root_expr == NULL) {
		std::cout << "Root expression was null! Ahhhhhhhhhhhhh!" << std::endl;
		return;
	}

	std::string error_str;
	llvm::Module* module = new llvm::Module("top", llvm::getGlobalContext());
	llvm::ExecutionEngine* execution_engine = llvm::EngineBuilder(module).setErrorStr(&error_str).setEngineKind(llvm::EngineKind::JIT).create();
	CodeGen code_gen(module);

	std::cout << "execution engine " << execution_engine << std::endl;
	if (execution_engine == NULL) {
		std::cout << "Unable to create execution engine." << std::endl;
		return;
	}

	NFunction main_fn(root_expr, NType::int_ty());

	// llvm::Value* root_val = root_expr->gen_code(&code_gen);

	// std::cout << "Root val code:" << std::endl;
	// root_val->dump();

	llvm::Function* main_fn_val = (llvm::Function*) main_fn.gen_code(&code_gen);
	std::cout << "Main fn code:" << std::endl;
	main_fn_val->dump();
	void* fn_ptr = execution_engine->getPointerToFunction(main_fn_val);
	int32_t (*fn_ptr_native)() = (int32_t (*)()) fn_ptr;
	int32_t ret = fn_ptr_native();
	std::cout << "Main fn at " << fn_ptr << "; executed: " << ret << std::endl;
}

int main(int argc, char* argv[]) {
	llvm::InitializeNativeTarget();
	std::cout << "Started." << std::endl;
	if (argc > 1) {
		std::string str = "";
		std::ifstream f(argv[1]);
		if (f.is_open()) {
			std::string line;
			while (std::getline(f, line)) {
				str += line + "\n";
			}
		}
		std::cout << "Reading from file, contents:\n" << str << std::endl;
		run_code(str.c_str());
	} else {
		std::string line;
		while (true) {
			std::cout << "> ";
			if (!std::getline(std::cin, line)) {
				break;
			}
			run_code(line.c_str());
		}
	}
	std::cout << "Done." << std::endl;
	return 0;
}
