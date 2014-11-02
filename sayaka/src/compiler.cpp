#include "compiler.h"

#include <llvm/Support/TargetSelect.h>
#include "parser.h"
#include "lexer.h"

Compiler::Compiler() {
	return;
}

Compiler::~Compiler() {
	return;
}

ASTNode* Compiler::parse(std::string code) {
	ASTNode* root;
	yyscan_t scanner;
	YY_BUFFER_STATE state;
	// (http://flex.sourceforge.net/manual/Reentrant-Overview.html)
	if (yylex_init(&scanner)) {
		return NULL;
	}
	state = yy_scan_string(code.c_str(), scanner);
	if (yyparse(&root, scanner)) {
		return NULL;
	}
	yy_delete_buffer(state, scanner);
	yylex_destroy(scanner);
	return root;
}

void Compiler::run_code(ASTNode* root) {
	return;
}

void Compiler::initialize() {
	llvm::InitializeNativeTarget();
}

/*

	CodeGen code_gen;

	NFunction main_fn(root_expr, ASTType::int_ty());

	llvm::Function* main_fn_val = (llvm::Function*) main_fn.gen_code(&code_gen);
	std::cout << "Main fn code:" << std::endl;
	main_fn_val->dump();
	void* fn_ptr = code_gen.execution_engine->getPointerToFunction(main_fn_val);
	int32_t ret = ((int32_t (*)()) fn_ptr)();
	std::cout << "Main fn at " << fn_ptr << "; executed: " << ret << std::endl;
	*/
