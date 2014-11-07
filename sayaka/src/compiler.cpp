#include "compiler.h"

#include <llvm/Support/TargetSelect.h>
#include "parser.h"
#include "lexer.h"
#include "ast_node.h"

Compiler::Compiler() {
	this->execution_engine = llvm::EngineBuilder(this->code_gen_context.module).setErrorStr(&this->execution_engine_error_str).setEngineKind(llvm::EngineKind::JIT).create();
}

Compiler::~Compiler() {
	delete this->execution_engine;
}

ASTNode* Compiler::parse(std::string code) {
	ASTNode* root;
	yyscan_t scanner;
	YY_BUFFER_STATE buffer;
	// (http://flex.sourceforge.net/manual/Reentrant-Overview.html)
	if (yylex_init(&scanner)) {
		return NULL;
	}
	buffer = yy_scan_string(code.c_str(), scanner);
	yyparse(&root, scanner);
	yy_delete_buffer(buffer, scanner);
	yylex_destroy(scanner);
	return root;
}

void Compiler::run_code(ASTNode* root) {
	ASTNodeFunction main_fn(root, "Int");
	main_fn.pass_types(&this->code_gen_context, NULL);
	llvm::Function* main_fn_val = (llvm::Function*) main_fn.gen_code(&this->code_gen_context);
	std::cout << "Main fn code:" << std::endl;
	main_fn_val->dump();
	void* fn_ptr = this->execution_engine->getPointerToFunction(main_fn_val);
	int32_t ret = ((int32_t (*)()) fn_ptr)();
	std::cout << "Main fn at " << fn_ptr << "; executed: " << ret << std::endl;
}

void Compiler::initialize() {
	llvm::InitializeNativeTarget();
}
